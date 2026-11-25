#include "Enemy/AI/EnemyAIProfile.h"
#include "Enemy/AI/MonsterArchetypeRow.h"

namespace
{
	TProfileValue<bool> ProfileResolveOverride(EEnemyOverride value, bool appearanceDefault) // 외형 기본값으로 원형을 덮어쓰는 함수
	{
		TProfileValue<bool> result;
		switch (value)
		{
		case EEnemyOverride::EOVR_On:  result.Set(true, EProfileSource::Archetype); break;
		case EEnemyOverride::EOVR_Off: result.Set(false, EProfileSource::Archetype); break;
		default:                       result.Set(appearanceDefault, EProfileSource::Appearance); break;
		}
		return result;
	}

	TProfileValue<float> ProfileResolveRange(float archetypeValue, float roleDefault) // 거리 구간을 설정하는 함수
	{
		TProfileValue<float> result;
		if (archetypeValue > 0.f)
			result.Set(archetypeValue, EProfileSource::Archetype);
		else
			result.Set(roleDefault, EProfileSource::Common);
		return result;
	}

	int32 ProfileGetFrenzyConvulsionEvery(EEnemyTemperament temperament) // 성향에 따른 경련 기본 주기를 반환하는 함수
	{
		switch (temperament)
		{
		case EEnemyTemperament::ETMP_Calm:       return 2;
		case EEnemyTemperament::ETMP_Aggressive: return 4;
		default:                                 return 3;
		}
	}

	const TCHAR* ProfileGetGradeName(EMonsterGrade grade) // 몬스터 등급 이름을 반환하는 함수
	{
		switch (grade)
		{
		case EMonsterGrade::EGRD_Elite:     return TEXT("엘리트");
		case EMonsterGrade::EGRD_FieldBoss: return TEXT("필드보스");
		case EMonsterGrade::EGRD_MainBoss:  return TEXT("메인보스");
		default:                            return TEXT("일반");
		}
	}

	const TCHAR* ProfileGetAppearanceName(EMonsterAppearance appearance) // 몬스터 외형 이름을 반환하는 함수
	{
		switch (appearance)
		{
		case EMonsterAppearance::EAPP_Humanoid: return TEXT("인간형");
		case EMonsterAppearance::EAPP_Beast:    return TEXT("짐승형");
		case EMonsterAppearance::EAPP_Avian:    return TEXT("조류형");
		case EMonsterAppearance::EAPP_Reptile:  return TEXT("파충형");
		case EMonsterAppearance::EAPP_Skeleton: return TEXT("해골형");
		default:                                return TEXT("기형형");
		}
	}

	const TCHAR* ProfileGetTemperamentName(EEnemyTemperament temperament) // 몬스터 성향 이름을 반환하는 함수
	{
		switch (temperament)
		{
		case EEnemyTemperament::ETMP_Calm:       return TEXT("차분");
		case EEnemyTemperament::ETMP_Aggressive: return TEXT("공격적");
		default:                                 return TEXT("기본");
		}
	}

	const TCHAR* ProfileGetReactionName(EEnemyReactionType reaction) // 몬스터 반응 이름을 반환하는 함수
	{
		switch (reaction)
		{
		case EEnemyReactionType::ERT_Guard: return TEXT("가드");
		case EEnemyReactionType::ERT_Dodge: return TEXT("회피");
		default:                            return TEXT("카운터");
		}
	}

	const TCHAR* ProfileGetIdleTypeName(EEnemyIdleType idleType) // 몬스터 대기 상태 이름을 반환하는 함수
	{
		switch (idleType)
		{
		case EEnemyIdleType::EIT_Patrol:      return TEXT("순찰");
		case EEnemyIdleType::EIT_Wander:      return TEXT("배회");
		case EEnemyIdleType::EIT_Circle:      return TEXT("맴돌기");
		case EEnemyIdleType::EIT_ScratchWall: return TEXT("벽 긁기");
		case EEnemyIdleType::EIT_Tremble:     return TEXT("웅크려 떪");
		case EEnemyIdleType::EIT_PeerCrack:   return TEXT("균열 들여다보기");
		case EEnemyIdleType::EIT_Bow:         return TEXT("헛절");
		case EEnemyIdleType::EIT_Ambush:      return TEXT("매복");
		case EEnemyIdleType::EIT_PlayDead:    return TEXT("시체 흉내");
		default:                              return TEXT("멈춰 섬");
		}
	}

	const TCHAR* ProfileOX(bool bValue)
	{
		return bValue ? TEXT("O") : TEXT("X");
	}
}

const TCHAR* EnemyAIProfile::GetSourceName(EProfileSource source) // 몬스터 출처 이름을 반환하는 함수
{
	switch (source)
	{
	case EProfileSource::Grade:       return TEXT("등급");
	case EProfileSource::Appearance:  return TEXT("외형");
	case EProfileSource::Temperament: return TEXT("성향");
	case EProfileSource::Archetype:   return TEXT("원형");
	case EProfileSource::Placement:   return TEXT("배치");
	default:                          return TEXT("공통");
	}
}

bool EnemyAIProfile::GetAppearancePackAttacker(EMonsterAppearance appearance) // 외형에 따른 무리 공격 여부를 반환하는 함수
{
	return appearance == EMonsterAppearance::EAPP_Beast || appearance == EMonsterAppearance::EAPP_Avian;
}

bool EnemyAIProfile::GetAppearanceDodgeRoll(EMonsterAppearance appearance) // 외형에 따른 반응 회부 여부를 반환하는 함수
{
	return appearance == EMonsterAppearance::EAPP_Humanoid;
}

FMonsterProfile EnemyAIProfile::Resolve(const FMonsterArchetypeRow& archetype, EEnemyIdleType placementIdleType)
{
	FMonsterProfile result;
	result.archetype = &archetype;

	/* 등급 설정 */
	const bool bBoss = archetype.grade == EMonsterGrade::EGRD_FieldBoss || archetype.grade == EMonsterGrade::EGRD_MainBoss;
	result.bPermanentDeath.Set(bBoss, EProfileSource::Grade);
	result.bFrenzyAttackSet.Set(archetype.grade != EMonsterGrade::EGRD_Normal, EProfileSource::Grade);

	/* 외형 및 원형 설정 */
	result.bPackAttacker = ProfileResolveOverride(archetype.packAttacker, GetAppearancePackAttacker(archetype.appearance));

	/* 성향 설정 */
	if (archetype.temperament != EEnemyTemperament::ETMP_Default)
		result.temperament.Set(archetype.temperament, EProfileSource::Archetype);
	else
		result.temperament.Set(result.bPackAttacker.value ? EEnemyTemperament::ETMP_Aggressive : EEnemyTemperament::ETMP_Normal, EProfileSource::Appearance);

	/* 반응 회피 설정 */
	if (archetype.bCanGuard)
		result.bCanDodgeRoll.Set(false, EProfileSource::Archetype);
	else
		result.bCanDodgeRoll = ProfileResolveOverride(archetype.canDodgeRoll, GetAppearanceDodgeRoll(archetype.appearance));

	/* 반응 방식 설정 */
	if (archetype.bCanGuard)
		result.reaction.Set(EEnemyReactionType::ERT_Guard, EProfileSource::Archetype);
	else if (result.bCanDodgeRoll.value)
		result.reaction.Set(EEnemyReactionType::ERT_Dodge, result.bCanDodgeRoll.source);
	else
		result.reaction.Set(EEnemyReactionType::ERT_Counter, result.bCanDodgeRoll.source);

	/* 거리 구간 설정 */
	const bool bRanged = archetype.combatRole == EEnemyCombatRole::ECR_Ranged;
	result.nearRange = ProfileResolveRange(archetype.nearRange, bRanged ? 3.f : 2.5f);
	result.midRange = ProfileResolveRange(archetype.midRange, bRanged ? 12.f : 6.f);
	if (bRanged)
		result.farRange = ProfileResolveRange(archetype.farRange, 20.f);
	else
		result.farRange.Set(0.f, EProfileSource::Common);

	/* 추적 거리 설정 */
	result.leash.Set(archetype.leashFromHome, EProfileSource::Archetype);
	result.maxLeash.Set(archetype.leashFromHome + MaxLeashExtra, EProfileSource::Common);

	/* 광증 확정 경련 주기 설정 */
	if (!archetype.bHasFrenzy)
		result.frenzyConvulsionEvery.Set(0, EProfileSource::Archetype);
	else if (archetype.frenzyConvulsionEvery > 0)
		result.frenzyConvulsionEvery.Set(archetype.frenzyConvulsionEvery, EProfileSource::Archetype);
	else
		result.frenzyConvulsionEvery.Set(ProfileGetFrenzyConvulsionEvery(result.temperament.value), EProfileSource::Temperament);

	/* 배치 설정 */
	if (placementIdleType != EEnemyIdleType::EIT_None)
		result.idleType.Set(placementIdleType, EProfileSource::Placement);
	else if (archetype.defaultIdleType != EEnemyIdleType::EIT_None)
		result.idleType.Set(archetype.defaultIdleType, EProfileSource::Archetype);
	else
		result.idleType.Set(EEnemyIdleType::EIT_Stand, EProfileSource::Common);

	return result;
}

TArray<FString> EnemyAIProfile::DescribeLines(const FMonsterProfile& profile)
{
	TArray<FString> lines;
	if (!profile.IsValid())
	{
		lines.Add(TEXT("원형 없음"));
		return lines;
	}

	const FMonsterArchetypeRow& archetype = *profile.archetype;

	lines.Add(FString::Printf(TEXT("정체 %s · %s · %s · %s"),
		*archetype.monsterId,
		ProfileGetGradeName(archetype.grade),
		ProfileGetAppearanceName(archetype.appearance),
		archetype.combatRole == EEnemyCombatRole::ECR_Ranged ? TEXT("원거리") : TEXT("근거리")));

	lines.Add(FString::Printf(TEXT("몸 HP %.0f · 강인도 %.0f · 날아감 면역 %s · 약점 %d개 ← 원형"),
		archetype.maxHP, archetype.toughness, ProfileOX(archetype.bKnockbackImmune), archetype.weakPointCount));

	lines.Add(FString::Printf(TEXT("성향 %s ← %s · 무리 공격 %s ← %s"),
		ProfileGetTemperamentName(profile.temperament.value), GetSourceName(profile.temperament.source),
		ProfileOX(profile.bPackAttacker.value), GetSourceName(profile.bPackAttacker.source)));

	lines.Add(FString::Printf(TEXT("반응 %s ← %s · 반응 회피 %s ← %s · 대기 %s ← %s"),
		ProfileGetReactionName(profile.reaction.value), GetSourceName(profile.reaction.source),
		ProfileOX(profile.bCanDodgeRoll.value), GetSourceName(profile.bCanDodgeRoll.source),
		ProfileGetIdleTypeName(profile.idleType.value), GetSourceName(profile.idleType.source)));

	const FString farText = profile.farRange.value > 0.f
		? FString::Printf(TEXT(" · 멀리 %.1f ← %s"), profile.farRange.value, GetSourceName(profile.farRange.source))
		: FString();

	lines.Add(FString::Printf(TEXT("구간 가까이 %.1f ← %s · 중간 %.1f ← %s%s · 추적 %.0f ← %s · 최대 %.0f ← %s"),
		profile.nearRange.value, GetSourceName(profile.nearRange.source),
		profile.midRange.value, GetSourceName(profile.midRange.source),
		*farText,
		profile.leash.value, GetSourceName(profile.leash.source),
		profile.maxLeash.value, GetSourceName(profile.maxLeash.source)));

	const FString frenzyText = profile.frenzyConvulsionEvery.value > 0
		? FString::Printf(TEXT("광증 경련 %d회마다 ← %s · 광증 방식 %s ← %s"),
			profile.frenzyConvulsionEvery.value, GetSourceName(profile.frenzyConvulsionEvery.source),
			profile.bFrenzyAttackSet.value ? TEXT("전용 공격 세트") : TEXT("예고 ×0.6 · 추적 없음"),
			GetSourceName(profile.bFrenzyAttackSet.source))
		: FString::Printf(TEXT("광증 없음 ← %s"), GetSourceName(profile.frenzyConvulsionEvery.source));

	lines.Add(FString::Printf(TEXT("%s · 리셋 %s ← %s"),
		*frenzyText,
		profile.bPermanentDeath.value ? TEXT("영구 사망") : TEXT("부활"),
		GetSourceName(profile.bPermanentDeath.source)));

	return lines;
}