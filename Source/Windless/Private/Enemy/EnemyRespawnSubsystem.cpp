#include "Enemy/EnemyRespawnSubsystem.h"
#include "Enemy/EnemySpawner.h"
#include "Windless/DebugMacro.h"

void UEnemyRespawnSubsystem::RegisterSpawner(AEnemySpawner* spawner)
{
	if (spawner)
		spawners.AddUnique(spawner);
}

void UEnemyRespawnSubsystem::UnregisterSpawner(AEnemySpawner* spawner)
{
	spawners.RemoveAll([spawner](const TWeakObjectPtr<AEnemySpawner>& entry)
		{
			return !entry.IsValid() || entry.Get() == spawner;
		});
}

int32 UEnemyRespawnSubsystem::RespawnAll(bool bIncludeAlive)
{
	int32 count = 0;

	for (const TWeakObjectPtr<AEnemySpawner>& spawner : spawners)
	{
		if (!spawner.IsValid() || spawner->IsBossSpawner())
			continue;

		if (spawner->IsEnemyAlive() && !bIncludeAlive)
			continue;

		spawner->RespawnEnemy(bIncludeAlive);
		++count;
	}

	UE_LOG(LogWindless, Log, TEXT("[리스폰] 몬스터 %d마리 재배치 (등록 %d)"), count, spawners.Num());
	return count;
}

void UEnemyRespawnSubsystem::PrintDebugInfo() const
{
	UE_LOG(LogWindless, Warning, TEXT("===== [리스폰] 스포너 %d개 ====="), spawners.Num());

	for (const TWeakObjectPtr<AEnemySpawner>& spawner : spawners)
	{
		if (!spawner.IsValid())
			continue;

		UE_LOG(LogWindless, Warning, TEXT("  %-28s %s%s"),
			*spawner->GetName(),
			spawner->IsEnemyAlive() ? TEXT("생존") : TEXT("사망"),
			spawner->IsBossSpawner() ? TEXT(" [보스]") : TEXT(""));
	}
}

bool UEnemyRespawnSubsystem::DoesSupportWorldType(const EWorldType::Type worldType) const
{
	return worldType == EWorldType::Game || worldType == EWorldType::PIE;
}
