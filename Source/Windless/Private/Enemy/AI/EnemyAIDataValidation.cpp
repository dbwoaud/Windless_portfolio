#include "Enemy/AI/EnemyAIDataValidation.h"
#include "Enemy/AI/EnemyAttackRow.h"
#include "Enemy/AI/MonsterArchetypeRow.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "Engine/DataTable.h"

namespace
{
	constexpr float ValidationEpsilon = 1e-4f;   // CSV 소수 비교 오차
	constexpr float MinWindup = 0.40f;           // V5 예고 하한
	constexpr float NoStaggerWindup = 0.50f;     // V6, V16 경직 가능 예고
	constexpr float MinHoldTime = 0.45f;         // V4 엇박 멈춤 하한
	constexpr float MinHoldReleaseTime = 0.45f;  // V11 멈춤 뒤 판정까지 하한
	constexpr float MaxDodgeChance = 0.5f;       // V1 반응 회피 확률 상한
	constexpr int32 MaxLightHits = 3;            // V7 약공격 최대 타수
	constexpr int32 MinEliteAttacks = 4;         // V14 엘리트 이상 공격 수

	bool ValidationLess(float a, float b) { return a < b - ValidationEpsilon; }
	bool ValidationGreater(float a, float b) { return a > b + ValidationEpsilon; }

	TCHAR ValidationDigit(int32 value)
	{
		return static_cast<TCHAR>(TEXT('0') + value);
	}

	const TCHAR* ValidationAppearanceCode(EMonsterAppearance appearance)
	{
		switch (appearance)
		{
		case EMonsterAppearance::EAPP_Humanoid: return TEXT("HM");
		case EMonsterAppearance::EAPP_Beast:    return TEXT("BS");
		case EMonsterAppearance::EAPP_Avian:    return TEXT("AV");
		case EMonsterAppearance::EAPP_Reptile:  return TEXT("RP");
		case EMonsterAppearance::EAPP_Skeleton: return TEXT("SK");
		default:                                return TEXT("AB");
		}
	}
}

void FEnemyAIValidationResult::AddError(const TCHAR* tag, const FString& owner, const FString& message)
{
	errors.Add(FString::Printf(TEXT("%s %s: %s"), tag, *owner, *message));
}

void FEnemyAIValidationResult::AddWarning(const TCHAR* tag, const FString& owner, const FString& message)
{
	warnings.Add(FString::Printf(TEXT("%s %s: %s"), tag, *owner, *message));
}

void FEnemyAIValidationResult::AppendTo(TArray<FString>& outProblems) const
{
	outProblems.Append(errors);
	for (const FString& warning : warnings)
		outProblems.Add(TEXT("(경고) ") + warning);
}

#if WITH_EDITOR
EDataValidationResult FEnemyAIValidationResult::ReportTo(FDataValidationContext& context) const
{
	for (const FString& error : errors)
		context.AddError(FText::FromString(error));
	for (const FString& warning : warnings)
		context.AddWarning(FText::FromString(warning));

	return HasErrors() ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

void EnemyAIDataValidation::ValidateAttack(const FEnemyAttackRow& attack, const FString& owner, const FEnemyAIReferenceValues& reference, FEnemyAIValidationResult& out)
{
	const int32 hits = attack.GetHitCount();
	const bool bLight = attack.attackClass == EEnemyAttackClass::EATC_Light;

	/* 배열 길이 */
	if (hits == 0)
	{
		out.AddError(TEXT("[배열]"), owner, TEXT("hitRanges가 비어 있음 (타수 0)"));
		return;
	}
	if (attack.windups.Num() != hits)
		out.AddError(TEXT("[배열]"), owner, FString::Printf(TEXT("windups %d개 ≠ 타수 %d"), attack.windups.Num(), hits));
	if (attack.activeTimes.Num() > 0 && attack.activeTimes.Num() != hits)
		out.AddError(TEXT("[배열]"), owner, FString::Printf(TEXT("activeTimes %d개 ≠ 타수 %d"), attack.activeTimes.Num(), hits));
	if (attack.trackProfiles.Num() > 0 && attack.trackProfiles.Num() != hits)
		out.AddError(TEXT("[배열]"), owner, FString::Printf(TEXT("trackProfiles %d개 ≠ 타수 %d"), attack.trackProfiles.Num(), hits));
	if (attack.hitIntervals.Num() != hits - 1)
		out.AddError(TEXT("[배열]"), owner, FString::Printf(TEXT("hitIntervals %d개 ≠ 타수 - 1 (%d)"), attack.hitIntervals.Num(), hits - 1));

	/* V7 타수 */
	if (bLight && hits > MaxLightHits)
		out.AddError(TEXT("[V7 타수]"), owner, FString::Printf(TEXT("약공격은 최대 %d타 (지금 %d타)"), MaxLightHits, hits));
	if (!bLight && hits != 1)
		out.AddError(TEXT("[V7 타수]"), owner, FString::Printf(TEXT("강공격은 1타 (지금 %d타)"), hits));

	/* V5 예고 하한 */
	for (int32 i = 0; i < attack.windups.Num(); ++i)
	{
		if (ValidationLess(attack.windups[i], MinWindup))
			out.AddError(TEXT("[V5 예고]"), owner, FString::Printf(TEXT("%d타 예고 %.2f < %.2f"), i + 1, attack.windups[i], MinWindup));
	}

	/* V4, V11 엇박 */
	if (attack.HasHold())
	{
		if (bLight)
			out.AddError(TEXT("[V4 엇박]"), owner, TEXT("엇박은 강공격만 쓸 수 있음"));
		if (ValidationLess(attack.windupHoldTime, MinHoldTime))
			out.AddError(TEXT("[V4 엇박]"), owner, FString::Printf(TEXT("멈춤 %.2f < %.2f"), attack.windupHoldTime, MinHoldTime));
		if (ValidationLess(attack.holdReleaseTime, MinHoldReleaseTime))
			out.AddError(TEXT("[V11 엇박]"), owner, FString::Printf(TEXT("멈춤 뒤 판정까지 %.2f < %.2f"), attack.holdReleaseTime, MinHoldReleaseTime));
		if (ValidationLess(attack.GetWindup(0), attack.windupHoldTime + attack.holdReleaseTime))
			out.AddError(TEXT("[엇박]"), owner, FString::Printf(TEXT("예고 %.2f < 멈춤 %.2f + 내려옴 %.2f"), attack.GetWindup(0), attack.windupHoldTime, attack.holdReleaseTime));
	}

	/* V12 타 사이 간격 >= 다음 타 예고 */
	for (int32 i = 0; i + 1 < hits && attack.hitIntervals.IsValidIndex(i); ++i)
	{
		if (ValidationLess(attack.hitIntervals[i], attack.GetWindup(i + 1)))
			out.AddError(TEXT("[V12 간격]"), owner, FString::Printf(TEXT("%d→%d타 간격 %.2f < %d타 예고 %.2f"), i + 1, i + 2, attack.hitIntervals[i], i + 2, attack.GetWindup(i + 1)));
	}

	/* V9 예고 효과음 */
	if (attack.windupSound.IsNull())
		out.AddError(TEXT("[V9 효과음]"), owner, TEXT("예고 효과음(windupSound)이 비어 있음"));

	/* V16 약한 경직 확정 연타 위험 (경고) */
	for (int32 i = 1; i < hits; ++i)
	{
		const float interval = attack.hitIntervals.IsValidIndex(i - 1) ? attack.hitIntervals[i - 1] : 0.f;
		if (ValidationGreater(attack.impact, reference.playerToughness)
			&& !ValidationLess(attack.GetWindup(i), NoStaggerWindup)
			&& ValidationLess(interval, reference.playerWeakStaggerTime))
		{
			out.AddWarning(TEXT("[V16 확정 연타]"), owner, FString::Printf(TEXT("%d타 충격량 %.0f > 플레이어 강인도 %.0f · 예고 %.2f ≥ %.2f · 간격 %.2f < 약한 경직 %.2f"),
				i + 1, attack.impact, reference.playerToughness, attack.GetWindup(i), NoStaggerWindup, interval, reference.playerWeakStaggerTime));
		}
	}
}

void EnemyAIDataValidation::ValidateArchetype(const FMonsterArchetypeRow& archetype, const FString& owner, FAttackFinder findAttack, FEnemyAIValidationResult& out)
{
	const bool bNormal = archetype.grade == EMonsterGrade::EGRD_Normal;
	const bool bBoss = archetype.grade == EMonsterGrade::EGRD_FieldBoss || archetype.grade == EMonsterGrade::EGRD_MainBoss;

	/* V1, V15 몸 */
	if (ValidationGreater(archetype.dodgeChance, MaxDodgeChance))
		out.AddError(TEXT("[V1 회피]"), owner, FString::Printf(TEXT("반응 회피 확률 %.2f > %.2f"), archetype.dodgeChance, MaxDodgeChance));
	if (archetype.bCanGuard && archetype.canDodgeRoll == EEnemyOverride::EOVR_On)
		out.AddError(TEXT("[V15 회피]"), owner, TEXT("방패가 있으면 반응 회피를 켤 수 없음 (canDodgeRoll = On)"));

	/* 참조 - 공격 목록 및 광증 세트 */
	TArray<TPair<FName, const FEnemyAttackRow*>> attacks;
	auto collectAttacks = [&](const TArray<FName>& ids, const TCHAR* listName)
		{
			for (const FName& id : ids)
			{
				if (const FEnemyAttackRow* attack = findAttack(id))
					attacks.Emplace(id, attack);
				else
					out.AddError(TEXT("[참조]"), owner, FString::Printf(TEXT("%s의 %s가 DT_EnemyAttack에 없음"), listName, *id.ToString()));
			}
		};
	collectAttacks(archetype.attackIds, TEXT("attackIds"));
	collectAttacks(archetype.frenzyAttackSet, TEXT("frenzyAttackSet"));

	for (const FEnemySpecialCard& card : archetype.specialCards)
	{
		if (!archetype.attackIds.Contains(card.attackId) && !archetype.frenzyAttackSet.Contains(card.attackId))
			out.AddError(TEXT("[참조]"), owner, FString::Printf(TEXT("특수 카드 %s가 attackIds · frenzyAttackSet 어디에도 없음"), *card.attackId.ToString()));
	}

	/* V14 공격 수 (광증 세트 제외) */
	if (!bNormal && archetype.attackIds.Num() < MinEliteAttacks)
		out.AddError(TEXT("[V14 공격 수]"), owner, FString::Printf(TEXT("엘리트 이상은 공격 %d종 이상 (지금 %d종)"), MinEliteAttacks, archetype.attackIds.Num()));

	/* 광증 세트 (1장 등급, V17) */
	if (archetype.frenzyAttackSet.Num() > 0)
	{
		if (bNormal)
			out.AddError(TEXT("[광증]"), owner, TEXT("광증 전용 공격 세트는 엘리트 이상만 가질 수 있음"));
		else if (!archetype.bHasFrenzy)
			out.AddWarning(TEXT("[광증]"), owner, TEXT("bHasFrenzy가 꺼져 있는데 광증 세트가 있음"));
	}

	/* 등급별 공격 규칙 - 광증 세트 포함 (V17) */
	bool bUsesHold = false;
	bool bUsesMultiTracking = false;

	for (const TPair<FName, const FEnemyAttackRow*>& pair : attacks)
	{
		const FEnemyAttackRow& attack = *pair.Value;
		const FString attackOwner = FString::Printf(TEXT("%s/%s"), *owner, *pair.Key.ToString());

		bUsesHold |= attack.HasHold();
		bUsesMultiTracking |= attack.UsesMultiHitTracking();

		if (!bNormal)
			continue;

		if (attack.comboType == EEnemyComboType::ECT_Full) // V2
			out.AddError(TEXT("[V2 연타]"), attackOwner, TEXT("일반 등급은 끝까지 연타 금지"));

		for (int32 i = 1; i < attack.GetHitCount(); ++i) // V3
		{
			if (ValidationGreater(attack.hitRanges[i], attack.hitRanges[0]))
				out.AddError(TEXT("[V3 사거리]"), attackOwner, FString::Printf(TEXT("일반 등급 %d타 사거리 %.2f > 1타 %.2f"), i + 1, attack.hitRanges[i], attack.hitRanges[0]));
		}

		if (attack.HasHold()) // V4 (등급 쪽)
			out.AddError(TEXT("[V4 엇박]"), attackOwner, TEXT("일반 등급은 엇박 금지"));

		if (attack.attackClass == EEnemyAttackClass::EATC_Light && attack.bCanStaggerStrong) // V8
			out.AddError(TEXT("[V8 경직]"), attackOwner, TEXT("일반 등급 약공격은 강한 경직 불가"));

		if (attack.UsesMultiHitTracking()) // V13 (일반)
			out.AddError(TEXT("[V13 회피 방해]"), attackOwner, TEXT("일반 등급은 다단 추적 금지"));
	}

	/* V13 회피 방해 (엘리트 및 보스) */
	if (!bNormal && !bBoss)
	{
		if (bUsesHold && bUsesMultiTracking)
			out.AddError(TEXT("[V13 회피 방해]"), owner, TEXT("엘리트는 엇박 · 다단 추적 중 하나만 (둘 다 사용)"));
		else if (!bUsesHold && !bUsesMultiTracking)
			out.AddError(TEXT("[V13 회피 방해]"), owner, TEXT("엘리트는 엇박 · 다단 추적 중 하나가 필요 (둘 다 없음)"));
	}
	else if (bBoss && !(bUsesHold && bUsesMultiTracking))
	{
		out.AddError(TEXT("[V13 회피 방해]"), owner, TEXT("보스는 엇박 + 다단 추적 + 넓은 판정 (넓은 판정은 칸 미정 - 검사 안 함)"));
	}

	/* V10 ID 자릿값 (경고) */
	const FString expectedId = BuildExpectedMonsterId(archetype);
	if (!MatchesMonsterId(archetype.monsterId, expectedId))
		out.AddWarning(TEXT("[V10 ID]"), owner, FString::Printf(TEXT("%s ≠ 분류 칸 기준 %s_NN"), *archetype.monsterId, *expectedId));
}

void EnemyAIDataValidation::ValidateArchetypeUsingSettings(const FMonsterArchetypeRow& archetype, const FString& owner, FEnemyAIValidationResult& out)
{
	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const UDataTable* attackTable = settings ? settings->attackTable.LoadSynchronous() : nullptr;

	ValidateArchetype(archetype, owner, [attackTable](FName id) -> const FEnemyAttackRow*
		{
			return attackTable ? attackTable->FindRow<FEnemyAttackRow>(id, TEXT("ValidateArchetypeUsingSettings"), false) : nullptr;
		}, out);
}

FEnemyAIValidationResult EnemyAIDataValidation::ValidateAllTables(int32& outArchetypeCount, int32& outAttackCount)
{
	FEnemyAIValidationResult result;
	outArchetypeCount = 0;
	outAttackCount = 0;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const UDataTable* attackTable = settings ? settings->attackTable.LoadSynchronous() : nullptr;
	const UDataTable* archetypeTable = settings ? settings->archetypeTable.LoadSynchronous() : nullptr;
	const FEnemyAIReferenceValues reference = GetReferenceValues();

	if (!attackTable)
		result.AddError(TEXT("[설정]"), TEXT("Enemy AI"), TEXT("Attack Table이 지정되지 않음"));
	if (!archetypeTable)
		result.AddError(TEXT("[설정]"), TEXT("Enemy AI"), TEXT("Archetype Table이 지정되지 않음"));

	if (attackTable)
	{
		outAttackCount = attackTable->GetRowMap().Num();
		attackTable->ForeachRow<FEnemyAttackRow>(TEXT("ValidateAllTables"), [&](const FName& rowName, const FEnemyAttackRow& attack)
			{
				ValidateAttack(attack, rowName.ToString(), reference, result);
			});
	}

	if (archetypeTable)
	{
		outArchetypeCount = archetypeTable->GetRowMap().Num();
		archetypeTable->ForeachRow<FMonsterArchetypeRow>(TEXT("ValidateAllTables"), [&](const FName& rowName, const FMonsterArchetypeRow& archetype)
			{
				ValidateArchetype(archetype, rowName.ToString(), [attackTable](FName id) -> const FEnemyAttackRow*
					{
						return attackTable ? attackTable->FindRow<FEnemyAttackRow>(id, TEXT("ValidateAllTables"), false) : nullptr;
					}, result);
			});
	}

	return result;
}

FEnemyAIReferenceValues EnemyAIDataValidation::GetReferenceValues()
{
	FEnemyAIReferenceValues reference;
	if (const UEnemyAISettings* settings = UEnemyAISettings::Get())
	{
		reference.playerToughness = settings->playerReferenceToughness;
		reference.playerWeakStaggerTime = settings->playerWeakStaggerTime;
	}
	return reference;
}

FString EnemyAIDataValidation::BuildExpectedMonsterId(const FMonsterArchetypeRow& archetype)
{
	const TCHAR rangeDigit = archetype.rangeClass == EMonsterRangeClass::ERC_Melee ? TEXT('1')
		: (archetype.rangeClass == EMonsterRangeClass::ERC_Ranged ? TEXT('2') : TEXT('3'));
	const TCHAR gradeDigit = ValidationDigit(static_cast<int32>(archetype.grade) + 1);
	const TCHAR aggressionDigit = archetype.aggression == EMonsterAggression::EAGG_Aggressive ? TEXT('1') : TEXT('0');

	const int32 modeCount = (archetype.bAttackDirect ? 1 : 0) + (archetype.bAttackSupport ? 1 : 0) + (archetype.bAttackSummon ? 1 : 0);
	TCHAR modeDigit = TEXT('?');
	if (modeCount == 1)
		modeDigit = archetype.bAttackDirect ? TEXT('1') : (archetype.bAttackSupport ? TEXT('2') : TEXT('3'));

	const TCHAR moveDigit = archetype.moveClass == EMonsterMoveClass::EMC_Mobile ? TEXT('1') : TEXT('2');
	const TCHAR socialDigit = ValidationDigit(static_cast<int32>(archetype.sociality) + 1);

	return FString::Printf(TEXT("MON_%s_%c%c%c%c%c%c"), ValidationAppearanceCode(archetype.appearance),
		rangeDigit, gradeDigit, aggressionDigit, modeDigit, moveDigit, socialDigit);
}

bool EnemyAIDataValidation::MatchesMonsterId(const FString& actual, const FString& expected)
{
	if (actual.Len() < expected.Len())
		return false;

	for (int32 i = 0; i < expected.Len(); ++i)
	{
		if (expected[i] != TEXT('?') && actual[i] != expected[i])
			return false;
	}

	const FString suffix = actual.Mid(expected.Len());
	return suffix.IsEmpty() || (suffix.Len() >= 2 && suffix[0] == TEXT('_') && suffix.Mid(1).IsNumeric());
}