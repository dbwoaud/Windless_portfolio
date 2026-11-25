#include "Components/Enemy/EnemyActionSelector.h"
#include "Components/Enemy/EnemyAttackRunner.h"
#include "Enemy/AI/EnemyAIProfile.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "Enemy/AI/EnemyAttackRow.h"
#include "Enemy/AI/MonsterArchetypeRow.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Windless/DebugMacro.h"

namespace
{
	TAutoConsoleVariable<int32> CVarCardDebug(
		TEXT("wl.CardDebug"),
		1,
		TEXT("카드 선택 디버그: 0 끔 / 1 로그 / 2 로그 + 화면"),
		ECVF_Default);

	/* 카드 이름 */
	FString SelectorCardLabel(EEnemyCard card, FName attackId)
	{
		return attackId.IsNone()
			? FString(EnemyCardScore::GetCardName(card))
			: FString::Printf(TEXT("%s[%s]"), EnemyCardScore::GetCardName(card), *attackId.ToString());
	}
}

UEnemyActionSelector::UEnemyActionSelector()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyActionSelector::Setup(const FMonsterArchetypeRow& inArchetype, const FMonsterProfile& inProfile, const TArray<const FEnemyAttackRow*>& inAttacks, const TArray<FName>& inAttackIds)
{
	archetype = &inArchetype;
	attacks = inAttacks;
	attackIds = inAttackIds;

	role = inArchetype.combatRole;
	temperament = inProfile.temperament.value;
	bPackAttacker = inProfile.bPackAttacker.value;
	bFixedMover = inArchetype.moveClass == EMonsterMoveClass::EMC_Fixed;

	bounds.nearMax = inProfile.nearRange.value;
	bounds.midMax = inProfile.midRange.value;
	bounds.farMax = inProfile.farRange.value;
	bounds.hysteresis = inArchetype.rangeHysteresis;

	currentBand = EEnemyRangeBand::ERB_Near;
	attackReadyTime.Reset();
	restUntil = 0.f;
	lastPick = FCardLastPick();
	nextModifiers.Reset();
	cardReadyTime.Reset();
	forcedCard.Reset();


	/* 카드 표 */
	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	cardTable = settings ? settings->cardBaseTable.LoadSynchronous() : nullptr;
	if (!cardTable)
		UE_LOG(LogWindless, Warning, TEXT("[카드] %s 카드 표가 없음"), *GetNameSafe(GetOwner()));

	/* 플레이어 상태 배율 */
	playerStateTable = settings ? settings->playerStateModTable.LoadSynchronous() : nullptr;
	if (!playerStateTable)
		UE_LOG(LogWindless, Warning, TEXT("[카드] %s 플레이어 상태 배율 표가 없음"), *GetNameSafe(GetOwner()));

	/* 직전 행동 보정 */
	lastActTable = settings ? settings->lastActModTable.LoadSynchronous() : nullptr;
	if (!lastActTable)
		UE_LOG(LogWindless, Warning, TEXT("[카드] %s 직전 행동 보정 표가 없음"), *GetNameSafe(GetOwner()));

	if (settings)
	{
		playerRules.frontHalfAngle = settings->frontHalfAngle;
		playerRules.counterRange = settings->counterRange;
		playerRules.itemPunishRange = settings->itemPunishRange;
		playerRules.aimRange = settings->aimRange;
		playerRules.guardHoldTime = settings->guardHoldThreshold;
	}

	attackReach = 0.f;
	for (const FEnemyAttackRow* attack : attacks)
	{
		if (attack)
			attackReach = FMath::Max(attackReach, attack->GetRange());
	}
	bCounterReaction = inProfile.reaction.value == EEnemyReactionType::ERT_Counter;


	/* 예고 시작 */
	if (UEnemyAttackRunner* runner = GetOwner() ? GetOwner()->FindComponentByClass<UEnemyAttackRunner>() : nullptr)
	{
		runner->onAttackerChanged.RemoveAll(this);
		runner->onAttackerChanged.AddUObject(this, &UEnemyActionSelector::HandleAttackerChanged);
	}

	for (const FEnemySpecialCard& special : inArchetype.specialCards)
	{
		if (special.condition != EEnemySpecialCondition::ESC_None)
			UE_LOG(LogWindless, Log, TEXT("[카드] %s 위치 조건 특수 카드 %s는 단계 10에서 연결"), *GetNameSafe(GetOwner()), *special.attackId.ToString());
	}


}

FEnemyCardChoice UEnemyActionSelector::Select(const FEnemyCardContext& context)
{
	FEnemyCardChoice choice;
	if (!IsReady())
	{
		choice.bFallbackRest = true;
		return choice;
	}

	/* 거리 구간 */
	currentBand = EnemyCardScore::ResolveBand(context.distance, currentBand, bounds);

	/* 기본 점수 */
	const FCardBaseScores baseScores = EnemyCardScore::CollectBaseScores(cardTable, role, currentBand, temperament);
	FCardBaseInput input;
	input.role = role;
	input.band = currentBand;
	input.baseScores = &baseScores;
	input.overrides = &archetype->cardScoreOverrides;
	BuildCandidates(context, input);
	lastEntries = EnemyCardScore::BuildBaseScores(input);

	/* 상황 보정 */
	TArray<FCardModifier> modifiers = MoveTemp(nextModifiers);
	nextModifiers.Reset();
	if (bFixedMover)
	{
		const EEnemyCard moveCards[3] = { EEnemyCard::EC_SideStep, EEnemyCard::EC_BackOff, EEnemyCard::EC_Approach };
		for (const EEnemyCard moveCard : moveCards)
		{
			FCardModifier& modifier = modifiers.AddDefaulted_GetRef();
			modifier.card = moveCard;
			modifier.multiplier = 0.f;
			modifier.source = TEXT("고정형");
		}
	}

	/* 비공격 카드 쿨타임 */
	for (const TPair<EEnemyCard, float>& cooldown : cardReadyTime)
	{
		if (GetNow() >= cooldown.Value)
			continue;

		FCardModifier& modifier = modifiers.AddDefaulted_GetRef();
		modifier.card = cooldown.Key;
		modifier.multiplier = 0.f;
		modifier.source = TEXT("쿨타임");
	}

	/* 이동 카드 공간 검사 */
	auto blockForSpace = [&modifiers](EEnemyCard moveCard)
		{
			FCardModifier& modifier = modifiers.AddDefaulted_GetRef();
			modifier.card = moveCard;
			modifier.multiplier = 0.f;
			modifier.source = TEXT("공간 없음");
		};
	if (context.bSideStepBlocked)
		blockForSpace(EEnemyCard::EC_SideStep);
	if (context.bBackOffBlocked)
		blockForSpace(EEnemyCard::EC_BackOff);

	EnemyCardScore::CollectPlayerStateMods(playerStateTable, role, GetActivePlayerStates(context), modifiers);
	EnemyCardScore::ApplyModifiers(lastEntries, modifiers);

	/* 차단 및 조절 */
	FCardBlockInput block;
	block.lastPick = lastPick;
	block.repeatSuppression = archetype->repeatSuppression;
	block.bRestTime = GetNow() < restUntil;
	block.bPlayerStaggered = context.bPlayerStaggered;
	block.bLoneSurvivor = context.bLoneSurvivor;
	block.bFrenzy = context.bFrenzy;
	EnemyCardScore::ApplyBlocks(lastEntries, block);

	/* 동시 공격자에 따른 카드 비율 설정 */
	FCardRatioInput ratio;
	ratio.otherAttackers = context.otherAttackers;
	ratio.bPackAttacker = bPackAttacker;
	lastChances = EnemyCardScore::GetPickChances(lastEntries, ratio);
	const int32 picked = EnemyCardScore::PickEntry(lastEntries, ratio, random);

	if (lastEntries.IsValidIndex(picked))
	{
		const FCardEntry& entry = lastEntries[picked];
		choice.card = entry.card;
		choice.attackId = entry.attackId;
		choice.attackIndex = entry.attackId.IsNone() ? INDEX_NONE : attackIds.IndexOfByKey(entry.attackId);

		lastPick.bValid = true;
		lastPick.card = entry.card;
		lastPick.attackId = entry.attackId;
	}
	else
	{
		choice.card = EEnemyCard::EC_Rest;
		choice.bFallbackRest = true;
	}

	if (forcedCard.IsSet())
	{
		choice = FEnemyCardChoice();
		choice.card = forcedCard.GetValue();
		forcedCard.Reset();
		UE_LOG(LogWindless, Log, TEXT("[카드] %s 시험용 강제 카드: %s"), *GetNameSafe(GetOwner()), EnemyCardScore::GetCardName(choice.card));
	}

	LogSelection(context, choice, modifiers);
	return choice;
}

void UEnemyActionSelector::AddNextModifiers(const TArray<FCardModifier>& modifiers)
{
	nextModifiers.Append(modifiers);
}

void UEnemyActionSelector::RecordLastAct(EEnemyLastAct act)
{
	const TCHAR* actName = EnemyCardScore::GetLastActName(act);
	nextModifiers.RemoveAll([actName](const FCardModifier& modifier)
		{
			return FCString::Strcmp(modifier.source, actName) == 0;
		});

	const int32 before = nextModifiers.Num();
	EnemyCardScore::CollectLastActMods(lastActTable, act, nextModifiers);
	UE_LOG(LogWindless, Log, TEXT("[카드] %s 직전 행동: %s, 다음 선택 1회 보정 %d개"),
		*GetNameSafe(GetOwner()), EnemyCardScore::GetLastActName(act), nextModifiers.Num() - before);
}

void UEnemyActionSelector::NotifyAttackCompleted()
{
	restUntil = GetNow() + (archetype ? archetype->minAttackInterval : 0.f);
}

void UEnemyActionSelector::NotifyCardStarted(EEnemyCard card)
{
	if (card != EEnemyCard::EC_BackOff || role != EEnemyCombatRole::ECR_Ranged)
		return;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	cardReadyTime.Add(card, GetNow() + (settings ? settings->rangedBackOffCooldown : 4.f));
}

void UEnemyActionSelector::ResetCombatMemory()
{
	nextModifiers.Reset();
	lastPick = FCardLastPick();
	UE_LOG(LogWindless, Log, TEXT("[카드] %s 전투 이탈(직전 행동 보정 및 반복 억제 기록 초기화)"), *GetNameSafe(GetOwner()));
}

void UEnemyActionSelector::ForceNextCard(EEnemyCard card)
{
	forcedCard = card;
}

TArray<EPlayerAIState> UEnemyActionSelector::GetActivePlayerStates(const FEnemyCardContext& context) const
{
	FPlayerStateQuery query;
	query.player = context.player;
	query.distance = context.distance;
	query.angleFromFront = context.angleFromFront;
	query.attackReach = attackReach;
	query.role = role;
	query.bCounterReaction = bCounterReaction;
	return EnemyCardScore::GetActivePlayerStates(query, playerRules);
}

void UEnemyActionSelector::BeginPlay()
{
	Super::BeginPlay();
	random.Initialize(randomSeed != 0 ? randomSeed : FMath::Rand());
}

void UEnemyActionSelector::HandleAttackerChanged(UEnemyAttackRunner* runner, bool bIsRegistered)
{
	if (!bIsRegistered || !runner)
		return;

	const FName attackId = runner->GetCurrentAttackId();
	const FEnemyAttackRow* attack = FindAttack(attackId);
	if (attack && attack->cooldown > 0.f)
		attackReadyTime.Add(attackId, GetNow() + attack->cooldown);
}

void UEnemyActionSelector::BuildCandidates(const FEnemyCardContext& context, FCardBaseInput& outInput) const
{
	/* 일반 공격 카드 */
	for (int32 i = 0; i < attacks.Num(); ++i)
	{
		const FEnemyAttackRow* attack = attacks[i];
		const FName attackId = attackIds.IsValidIndex(i) ? attackIds[i] : NAME_None;
		if (!attack || IsSpecialAttack(attackId) || !IsAttackReady(attackId))
			continue;

		if (!EnemyCardScore::IsAttackInReach(*attack, context.distance, context.heightDelta))
			continue;

		FCardAttackOption& option = outInput.attacks.AddDefaulted_GetRef();
		option.attackId = attackId;
		option.card = attack->card;
		option.shareWeight = attack->shareWeight;
	}

	/* 특수 카드 */
	for (const FEnemySpecialCard& special : archetype->specialCards)
	{
		if (special.condition != EEnemySpecialCondition::ESC_None)
			continue;

		const FEnemyAttackRow* attack = FindAttack(special.attackId);
		if (!attack || !IsAttackReady(special.attackId) || !EnemyCardScore::IsAttackInReach(*attack, context.distance, context.heightDelta))
			continue;

		FCardSpecialOption& option = outInput.specials.AddDefaulted_GetRef();
		option.attackId = special.attackId;
		option.bundle = special.bundle;
		switch (currentBand)
		{
		case EEnemyRangeBand::ERB_Near: option.score = special.scoreNear; break;
		case EEnemyRangeBand::ERB_Mid:  option.score = special.scoreMid;  break;
		case EEnemyRangeBand::ERB_Far:  option.score = special.scoreFar;  break;
		default:                        option.score = 0.f;               break;
		}
	}
}

const FEnemyAttackRow* UEnemyActionSelector::FindAttack(FName attackId) const
{
	const int32 index = attackIds.IndexOfByKey(attackId);
	return attacks.IsValidIndex(index) ? attacks[index] : nullptr;
}

bool UEnemyActionSelector::IsSpecialAttack(FName attackId) const
{
	return archetype && archetype->specialCards.ContainsByPredicate([attackId](const FEnemySpecialCard& special) { return special.attackId == attackId; });
}

bool UEnemyActionSelector::IsAttackReady(FName attackId) const
{
	const float* readyTime = attackReadyTime.Find(attackId);
	return !readyTime || GetNow() >= *readyTime;
}

float UEnemyActionSelector::GetNow() const
{
	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

void UEnemyActionSelector::LogSelection(const FEnemyCardContext& context, const FEnemyCardChoice& choice, const TArray<FCardModifier>& modifiers) const
{
	const int32 level = CVarCardDebug.GetValueOnGameThread();
	if (level <= 0)
		return;

	TArray<FString> lines;
	const FString pickText = choice.bFallbackRest ? FString(TEXT("숨 고르기")) : SelectorCardLabel(choice.card, choice.attackId);
	lines.Add(FString::Printf(TEXT("[카드] %s %s %.1fm%s: %s"), *GetNameSafe(GetOwner()), EnemyCardScore::GetBandName(currentBand),
		context.distance, context.bPlayerStaggered ? TEXT(", 플레이어 경직") : TEXT(""), *pickText));

	if (modifiers.Num() > 0)
	{
		FString modifierText;
		for (const FCardModifier& modifier : modifiers)
		{
			const TCHAR* target = modifier.bWholeBundle
				? (modifier.bundle == EEnemyCardBundle::ECB_Attack ? TEXT("공격 묶음") : TEXT("간 보기 묶음"))
				: EnemyCardScore::GetCardName(modifier.card);
			modifierText += FString::Printf(TEXT("%s%s %s *%.1f"), modifierText.IsEmpty() ? TEXT("") : TEXT(", "), modifier.source, target, modifier.multiplier);
		}
		lines.Add(FString::Printf(TEXT("  보정: %s"), *modifierText));
	}

	for (int32 i = 0; i < lastEntries.Num(); ++i)
	{
		const FCardEntry& entry = lastEntries[i];
		FString notes;
		if (entry.bModifierCapped)
			notes += TEXT(" (상한)");
		if (entry.bRepeatSuppressed)
			notes += TEXT(" (반복)");
		if (entry.blockReason)
			notes += FString::Printf(TEXT(" (차단: %s)"), entry.blockReason);

		lines.Add(FString::Printf(TEXT("  %s 1. %.1f 2. *%.2f 3. %.1f: %.1f%%%s"), *SelectorCardLabel(entry.card, entry.attackId),
			entry.baseScore, entry.modifierScale, entry.finalScore, lastChances.IsValidIndex(i) ? lastChances[i] * 100.f : 0.f, *notes));
	}

	for (const FString& line : lines)
		UE_LOG(LogWindless, Log, TEXT("%s"), *line);

	if (level >= 2 && GEngine)
		GEngine->AddOnScreenDebugMessage(static_cast<uint64>(GetUniqueID()) + 0x30000000, 4.f, FColor::Cyan, FString::Join(lines, TEXT("\n")));
}
