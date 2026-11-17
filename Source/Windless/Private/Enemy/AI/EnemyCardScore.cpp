#include "Enemy/AI/EnemyCardScore.h"
#include "Enemy/AI/EnemyCardBaseRow.h"
#include "Enemy/AI/EnemyAttackRow.h"
#include "Engine/DataTable.h"

EEnemyRangeBand EnemyCardScore::ResolveBand(float distance, EEnemyRangeBand prevBand, const FCardBandBounds& bounds)
{
	const float edges[3] = { bounds.nearMax, bounds.midMax, bounds.farMax };
	const int32 edgeCount = bounds.farMax > 0.f ? 3 : 2;
	const int32 prev = static_cast<int32>(prevBand);
	const float hysteresis = FMath::Max(0.f, bounds.hysteresis);

	int32 band = 0;
	for (int32 i = 0; i < edgeCount; ++i)
	{
		const bool bBeyond = prev <= i ? distance >= edges[i] + hysteresis : distance > edges[i] - hysteresis;
		if (!bBeyond)
			break;

		band = i + 1;
	}
	return static_cast<EEnemyRangeBand>(band);
}

const TArray<EEnemyCard>& EnemyCardScore::GetRoleCards(EEnemyCombatRole role)
{
	static const TArray<EEnemyCard> meleeCards = {
		EEnemyCard::EC_LightAttack, EEnemyCard::EC_HeavyAttack, EEnemyCard::EC_Rush,
		EEnemyCard::EC_SideStep, EEnemyCard::EC_BackOff, EEnemyCard::EC_Approach, EEnemyCard::EC_Rest };

	static const TArray<EEnemyCard> rangedCards = {
		EEnemyCard::EC_Shoot, EEnemyCard::EC_MeleeAttack,
		EEnemyCard::EC_BackOff, EEnemyCard::EC_SideStep, EEnemyCard::EC_Approach, EEnemyCard::EC_Rest };

	return role == EEnemyCombatRole::ECR_Ranged ? rangedCards : meleeCards;
}

bool EnemyCardScore::IsAttackCard(EEnemyCard card)
{
	switch (card)
	{
	case EEnemyCard::EC_LightAttack:
	case EEnemyCard::EC_HeavyAttack:
	case EEnemyCard::EC_Rush:
	case EEnemyCard::EC_Shoot:
	case EEnemyCard::EC_MeleeAttack:
	case EEnemyCard::EC_Special:
		return true;
	default:
		return false;
	}
}

EEnemyCardBundle EnemyCardScore::GetCardBundle(EEnemyCard card)
{
	return IsAttackCard(card) ? EEnemyCardBundle::ECB_Attack : EEnemyCardBundle::ECB_Probe;
}

FCardBaseScores EnemyCardScore::CollectBaseScores(const UDataTable* table, EEnemyCombatRole role, EEnemyRangeBand band, EEnemyTemperament temperament)
{
	FCardBaseScores scores;
	if (!table)
		return scores;

	table->ForeachRow<FEnemyCardBaseRow>(TEXT("EnemyCardScore::CollectBaseScores"), [&](const FName& rowName, const FEnemyCardBaseRow& row)
		{
			if (row.role == role && row.band == band)
				scores.Add(row.card, row.GetScore(temperament));
		});
	return scores;
}

TArray<FCardEntry> EnemyCardScore::BuildBaseScores(const FCardBaseInput& input)
{
	TArray<FCardEntry> entries;

	for (const EEnemyCard card : GetRoleCards(input.role))
	{
		/* 원형 덮어쓰기 */
		const float* tableScore = input.baseScores ? input.baseScores->Find(card) : nullptr;
		float score = tableScore ? *tableScore : 0.f;
		if (input.overrides)
		{
			for (const FEnemyCardScoreOverride& scoreOverride : *input.overrides)
			{
				if (scoreOverride.card == card && scoreOverride.band == input.band)
					score = scoreOverride.score;
			}
		}

		/* 공격이 아닌 카드 */
		if (!IsAttackCard(card))
		{
			FCardEntry& entry = entries.AddDefaulted_GetRef();
			entry.card = card;
			entry.bundle = EEnemyCardBundle::ECB_Probe;
			entry.baseScore = score;
			continue;
		}

		/* 공격 카드 */
		float totalWeight = 0.f;
		int32 attackCount = 0;
		for (const FCardAttackOption& attack : input.attacks)
		{
			if (attack.card != card)
				continue;

			totalWeight += FMath::Max(0.f, attack.shareWeight);
			++attackCount;
		}

		if (attackCount == 0)
			continue;

		for (const FCardAttackOption& attack : input.attacks)
		{
			if (attack.card != card)
				continue;

			const float share = totalWeight > 0.f ? FMath::Max(0.f, attack.shareWeight) / totalWeight : 1.f / attackCount;
			FCardEntry& entry = entries.AddDefaulted_GetRef();
			entry.card = card;
			entry.attackId = attack.attackId;
			entry.bundle = EEnemyCardBundle::ECB_Attack;
			entry.baseScore = score * share;
		}
	}

	/* 원형 특수 카드 */
	for (const FCardSpecialOption& special : input.specials)
	{
		FCardEntry& entry = entries.AddDefaulted_GetRef();
		entry.card = EEnemyCard::EC_Special;
		entry.attackId = special.attackId;
		entry.bundle = special.bundle;
		entry.bSpecial = true;
		entry.baseScore = special.score;
	}

	for (FCardEntry& entry : entries)
	{
		entry.modifiedScore = entry.baseScore;
		entry.finalScore = entry.baseScore;
	}
	return entries;
}

float EnemyCardScore::GetBundleRatio(const TArray<FCardEntry>& entries, EEnemyCardBundle bundle)
{
	float total = 0.f;
	float bundleSum = 0.f;
	for (const FCardEntry& entry : entries)
	{
		total += entry.baseScore;
		if (entry.bundle == bundle)
			bundleSum += entry.baseScore;
	}
	return total > 0.f ? bundleSum / total : 0.f;
}

WINDLESS_API bool EnemyCardScore::IsAttackInReach(const FEnemyAttackRow& attack, float distance, float heightDelta)
{
	if (distance < attack.minRange || distance > attack.GetRange())
		return false;

	const float verticalAngle = FMath::RadiansToDegrees(FMath::Atan2(heightDelta, FMath::Max(distance, KINDA_SMALL_NUMBER)));
	return verticalAngle <= attack.attackAngleUp && verticalAngle >= -attack.attackAngleDown;
}

const TCHAR* EnemyCardScore::GetCardName(EEnemyCard card)
{
	switch (card)
	{
	case EEnemyCard::EC_LightAttack: return TEXT("약공격");
	case EEnemyCard::EC_HeavyAttack: return TEXT("강공격");
	case EEnemyCard::EC_Rush:        return TEXT("돌진");
	case EEnemyCard::EC_Shoot:       return TEXT("사격");
	case EEnemyCard::EC_MeleeAttack: return TEXT("근접 공격");
	case EEnemyCard::EC_SideStep:    return TEXT("옆걸음");
	case EEnemyCard::EC_BackOff:     return TEXT("뒤로 빠지기");
	case EEnemyCard::EC_Approach:    return TEXT("다가가기");
	case EEnemyCard::EC_Rest:        return TEXT("숨 고르기");
	case EEnemyCard::EC_Special:     return TEXT("특수");
	default:                         return TEXT("?");
	}
}

const TCHAR* EnemyCardScore::GetBandName(EEnemyRangeBand band)
{
	switch (band)
	{
	case EEnemyRangeBand::ERB_Near:       return TEXT("가까이");
	case EEnemyRangeBand::ERB_Mid:        return TEXT("중간");
	case EEnemyRangeBand::ERB_Far:        return TEXT("멀리");
	case EEnemyRangeBand::ERB_OutOfRange: return TEXT("사거리 밖");
	default:                              return TEXT("?");
	}
}

void EnemyCardScore::ApplyModifiers(TArray<FCardEntry>& entries, const TArray<FCardModifier>& modifiers, float maxProduct)
{
	for (FCardEntry& entry : entries)
	{
		float up = 1.f;
		float down = 1.f;
		bool bZero = false;
		for (const FCardModifier& modifier : modifiers)
		{
			if (!modifier.Matches(entry))
				continue;

			if (modifier.multiplier <= 0.f)
				bZero = true;
			else if (modifier.multiplier >= 1.f)
				up *= modifier.multiplier;
			else
				down *= modifier.multiplier;
		}

		// 1 이상인 보정끼리의 곱에만 상한을 걸고, 1 미만 보정은 그 뒤에 곱한다
		entry.bModifierCapped = up > maxProduct;
		entry.modifierScale = bZero ? 0.f : FMath::Min(up, maxProduct) * down;
		entry.modifiedScore = entry.baseScore * entry.modifierScale;
		entry.finalScore = entry.modifiedScore;
	}
}

void EnemyCardScore::ApplyBlocks(TArray<FCardEntry>& entries, const FCardBlockInput& block)
{
	for (FCardEntry& entry : entries)
	{
		entry.finalScore = entry.modifiedScore;
		entry.bRepeatSuppressed = false;
		entry.blockReason = nullptr;

		/* 반복 억제 */
		if (block.lastPick.IsSame(entry))
		{
			entry.finalScore *= block.repeatSuppression;
			entry.bRepeatSuppressed = true;
		}

		/* 상태 차단 */
		if (entry.bundle == EEnemyCardBundle::ECB_Attack)
		{
			if (block.bPlayerStaggered)
				entry.blockReason = TEXT("플레이어 경직");
			else if (block.bRestTime)
				entry.blockReason = TEXT("쉬는 시간");
			else if (block.bLoneSurvivor)
				entry.blockReason = TEXT("혼자 남은 적");
		}
		else if (block.bFrenzy && entry.card == EEnemyCard::EC_Rest)
		{
			entry.blockReason = TEXT("광증");
		}

		if (entry.blockReason)
			entry.finalScore = 0.f;
	}
}

float EnemyCardScore::GetAttackBundleShare(const FCardRatioInput& ratio)
{
	if (ratio.bPackAttacker || ratio.otherAttackers <= 1)
		return -1.f;

	return ratio.otherAttackers == 2 ? RatioTwoAttackers : RatioManyAttackers;
}

TArray<float> EnemyCardScore::GetPickChances(const TArray<FCardEntry>& entries, const FCardRatioInput& ratio)
{
	TArray<float> chances;
	chances.SetNumZeroed(entries.Num());

	float attackSum = 0.f;
	float probeSum = 0.f;
	for (const FCardEntry& entry : entries)
	{
		const float score = FMath::Max(0.f, entry.finalScore);
		if (entry.bundle == EEnemyCardBundle::ECB_Attack)
			attackSum += score;
		else
			probeSum += score;
	}

	const float total = attackSum + probeSum;
	if (total <= 0.f)
		return chances;

	const float attackShare = GetAttackBundleShare(ratio);
	const bool bAdjust = attackShare >= 0.f && attackSum > 0.f && probeSum > 0.f;

	for (int32 i = 0; i < entries.Num(); ++i)
	{
		const float score = FMath::Max(0.f, entries[i].finalScore);
		if (!bAdjust)
			chances[i] = score / total;
		else if (entries[i].bundle == EEnemyCardBundle::ECB_Attack)
			chances[i] = attackShare * score / attackSum;
		else
			chances[i] = (1.f - attackShare) * score / probeSum;
	}
	return chances;
}

int32 EnemyCardScore::PickEntry(const TArray<FCardEntry>& entries, const FCardRatioInput& ratio, FRandomStream& random)
{
	const TArray<float> chances = GetPickChances(entries, ratio);

	float roll = random.FRand();
	int32 lastValid = INDEX_NONE;
	for (int32 i = 0; i < chances.Num(); ++i)
	{
		if (chances[i] <= 0.f)
			continue;

		lastValid = i;
		roll -= chances[i];
		if (roll < 0.f)
			return i;
	}
	return lastValid;
}

TArray<EPlayerAIState> EnemyCardScore::GetActivePlayerStates(const FPlayerStateQuery& query, const FPlayerStateRules& rules)
{
	TArray<EPlayerAIState> states;
	const bool bFront = query.angleFromFront <= rules.frontHalfAngle;
	const bool bRear = query.angleFromFront >= 180.f - rules.frontHalfAngle;

	/* 휘두르기 시작(정면 3m) */
	if (query.player.bAttackBegin && bFront && query.distance <= rules.counterRange
		&& (query.role == EEnemyCombatRole::ECR_Ranged || query.bCounterReaction))
		states.Add(EPlayerAIState::EPS_AttackBegin);

	/* 헛방 후딜(적 사거리 안) */
	if (query.player.bWhiff && query.distance <= query.attackReach)
		states.Add(EPlayerAIState::EPS_Whiff);

	/* 회복이나 아이템 사용(3.5m 안) */
	if (query.player.bUsingItem && query.distance <= rules.itemPunishRange)
		states.Add(EPlayerAIState::EPS_ItemUse);

	/* 활 조준(8m 안) */
	if (query.player.bAiming && query.distance <= rules.aimRange)
		states.Add(EPlayerAIState::EPS_BowAim);

	/* 가드 2초 이상(가장 긴 근접 공격 사거리 안) */
	if (query.player.guardHoldTime >= rules.guardHoldTime && query.distance <= query.attackReach)
		states.Add(EPlayerAIState::EPS_GuardHold);

	/* 뒤에서 접근(후방 3m 안) */
	if (bRear && query.distance <= rules.counterRange)
		states.Add(EPlayerAIState::EPS_Behind);

	return states;
}

void EnemyCardScore::CollectPlayerStateMods(const UDataTable* table, EEnemyCombatRole role, const TArray<EPlayerAIState>& states, TArray<FCardModifier>& outModifiers)
{
	if (!table || states.Num() == 0)
		return;

	table->ForeachRow<FEnemyPlayerStateModRow>(TEXT("CollectPlayerStateMods"), [&](const FName& rowName, const FEnemyPlayerStateModRow& row)
		{
			if (row.role != role || !states.Contains(row.state))
				return;

			FCardModifier& modifier = outModifiers.AddDefaulted_GetRef();
			modifier.card = row.card;
			modifier.multiplier = row.multiplier;
			modifier.source = GetPlayerStateName(row.state);
		});
}

const TCHAR* EnemyCardScore::GetPlayerStateName(EPlayerAIState state)
{
	switch (state)
	{
	case EPlayerAIState::EPS_AttackBegin: return TEXT("휘두르기 시작");
	case EPlayerAIState::EPS_Whiff:       return TEXT("헛방");
	case EPlayerAIState::EPS_ItemUse:     return TEXT("아이템 사용");
	case EPlayerAIState::EPS_BowAim:      return TEXT("활 조준");
	case EPlayerAIState::EPS_GuardHold:   return TEXT("가드 유지");
	case EPlayerAIState::EPS_Behind:      return TEXT("등 뒤");
	default:                              return TEXT("?");
	}
}

void EnemyCardScore::CollectLastActMods(const UDataTable* table, EEnemyLastAct act, TArray<FCardModifier>& outModifiers)
{
	if (!table)
		return;

	table->ForeachRow<FEnemyLastActModRow>(TEXT("CollectLastActMods"), [&](const FName& rowName, const FEnemyLastActModRow& row)
		{
			if (row.act != act)
				return;

			FCardModifier& modifier = outModifiers.AddDefaulted_GetRef();
			modifier.card = row.card;
			modifier.bWholeBundle = row.bWholeBundle;
			modifier.bundle = row.bundle;
			modifier.multiplier = row.multiplier;
			modifier.source = GetLastActName(row.act);
		});
}

const TCHAR* EnemyCardScore::GetLastActName(EEnemyLastAct act)
{
	switch (act)
	{
	case EEnemyLastAct::ELA_LightHit:            return TEXT("약공격 적중");
	case EEnemyLastAct::ELA_HeavyHit:            return TEXT("강공격 적중");
	case EEnemyLastAct::ELA_ComboEnd:            return TEXT("연타 종료");
	case EEnemyLastAct::ELA_Whiff:               return TEXT("공격 빗나감");
	case EEnemyLastAct::ELA_Endured:             return TEXT("피격 버팀");
	case EEnemyLastAct::ELA_GuardSuccess:        return TEXT("가드 성공");
	case EEnemyLastAct::ELA_DodgeSuccess:        return TEXT("회피 성공");
	case EEnemyLastAct::ELA_GuardBreakRecovered: return TEXT("가드 브레이크 회복 직후");
	case EEnemyLastAct::ELA_StaggerRecovered:    return TEXT("경직 회복 직후");
	case EEnemyLastAct::ELA_ConvulsionEnded:     return TEXT("경련 완료 직후");
	case EEnemyLastAct::ELA_ComboCancelChase:    return TEXT("연타 취소(추격)");
	case EEnemyLastAct::ELA_ComboCancelWatch:    return TEXT("연타 취소(관망)");
	default:                                     return TEXT("?");
	}
}