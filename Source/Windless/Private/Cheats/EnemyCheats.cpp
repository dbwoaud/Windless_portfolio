#include "Cheats/EnemyCheats.h"
#include "Enemy/Enemy.h"
#include "Enemy/EnemyRespawnSubsystem.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "Enemy/AI/EnemyAIProfile.h"
#include "Enemy/AI/EnemyAIDataValidation.h"
#include "EngineUtils.h"
#include "Windless/DebugMacro.h"

void UEnemyCheats::EnmSpawners()
{
	if (UWorld* world = GetWorld())
	{
		if (UEnemyRespawnSubsystem* respawn = world->GetSubsystem<UEnemyRespawnSubsystem>())
			respawn->PrintDebugInfo();
	}
}

void UEnemyCheats::EnmRespawnEnemies(bool bIncludeAlive)
{
	if (UWorld* world = GetWorld())
	{
		if (UEnemyRespawnSubsystem* respawn = world->GetSubsystem<UEnemyRespawnSubsystem>())
			respawn->RespawnAll(bIncludeAlive);
	}
}

void UEnemyCheats::EnmData()
{
	/* 데이터 검사 */
	int32 archetypeCount = 0;
	int32 attackCount = 0;
	const FEnemyAIValidationResult result = EnemyAIDataValidation::ValidateAllTables(archetypeCount, attackCount);

	const FString summary = FString::Printf(TEXT("[AI 검사] 원형 %d종, 공격 %d종: 에러 %d, 경고 %d"),
		archetypeCount, attackCount, result.errors.Num(), result.warnings.Num());
	UE_LOG(LogWindless, Log, TEXT("%s"), *summary);
	for (const FString& error : result.errors)
		UE_LOG(LogWindless, Error, TEXT("%s"), *error);
	for (const FString& warning : result.warnings)
		UE_LOG(LogWindless, Warning, TEXT("%s"), *warning);

	/* 가장 가까운 적의 프로필 */
	const APawn* pawn = GetPlayerPawn();
	UWorld* world = GetWorld();
	AEnemy* nearest = nullptr;
	float nearestDistSq = TNumericLimits<float>::Max();
	if (pawn && world)
	{
		for (TActorIterator<AEnemy> it(world); it; ++it)
		{
			if (!it->GetProfile().IsValid())
				continue;

			const float distSq = FVector::DistSquared(it->GetActorLocation(), pawn->GetActorLocation());
			if (distSq < nearestDistSq)
			{
				nearestDistSq = distSq;
				nearest = *it;
			}
		}
	}

	TArray<FString> profileLines;
	if (nearest)
	{
		profileLines.Add(FString::Printf(TEXT("[AI 프로필] %s (%.1fm)"), *nearest->GetName(), FMath::Sqrt(nearestDistSq) / 100.f));
		profileLines.Append(EnemyAIProfile::DescribeLines(nearest->GetProfile()));
	}
	else
	{
		profileLines.Add(TEXT("[AI 프로필] 원형이 지정된 적이 없음"));
	}
	for (const FString& line : profileLines)
		UE_LOG(LogWindless, Log, TEXT("%s"), *line);

	/* 화면 표시 */
	if (!GEngine)
		return;

	constexpr float ScreenDuration = 10.f;
	constexpr int32 MaxScreenIssues = 12;
	int32 key = 400;
	int32 shownIssues = 0;

	GEngine->AddOnScreenDebugMessage(key++, ScreenDuration, result.HasErrors() ? FColor::Red : FColor::Green, summary);

	auto showIssue = [&](const FString& text, const FColor& color)
		{
			if (shownIssues >= MaxScreenIssues)
				return;
			GEngine->AddOnScreenDebugMessage(key++, ScreenDuration, color, text);
			++shownIssues;
		};
	for (const FString& error : result.errors)
		showIssue(error, FColor::Red);
	for (const FString& warning : result.warnings)
		showIssue(warning, FColor::Yellow);

	const int32 totalIssues = result.errors.Num() + result.warnings.Num();
	if (totalIssues > MaxScreenIssues)
		GEngine->AddOnScreenDebugMessage(key++, ScreenDuration, FColor::Silver, FString::Printf(TEXT("... 외 %d건 (출력 로그 참고)"), totalIssues - MaxScreenIssues));

	for (const FString& line : profileLines)
		GEngine->AddOnScreenDebugMessage(key++, ScreenDuration, FColor::Cyan, line);
}

void UEnemyCheats::EnmForceCard(const FString& cardName)
{
	struct FCardName { const TCHAR* name; EEnemyCard card; };
	static const FCardName names[] = {
		{ TEXT("rest"),     EEnemyCard::EC_Rest },
		{ TEXT("side"),     EEnemyCard::EC_SideStep },
		{ TEXT("back"),     EEnemyCard::EC_BackOff },
		{ TEXT("approach"), EEnemyCard::EC_Approach },
	};

	UWorld* world = GetWorld();
	if (!world)
		return;

	for (const FCardName& entry : names)
	{
		if (!cardName.Equals(entry.name, ESearchCase::IgnoreCase))
			continue;

		int32 count = 0;
		for (TActorIterator<AEnemy> it(world); it; ++it)
		{
			it->DebugForceNextCard(entry.card);
			++count;
		}
		UE_LOG(LogWindless, Log, TEXT("[카드] 시험용 강제 카드 %s: 적 %d마리의 다음 선택"), entry.name, count);
		return;
	}

	UE_LOG(LogWindless, Warning, TEXT("[카드] 사용법: WLForceCard rest | side | back | approach"));
}