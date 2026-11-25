#include "Enemy/Boss.h"
#include "HUD/WindlessHUD.h"
#include "HUD/WindlessOverlay.h"
#include "Components/AttributeComponent.h"
#include "Windless/DebugMacro.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

float ABoss::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!bBossHealthBarVisible)
		return applied;

	if (UWindlessOverlay* overlay = ResolveOverlay())
		overlay->SetBossHealthPercent(attributes ? attributes->GetHealthPercent() : 0.f);

	return applied;
}

void ABoss::ShowBossHealthBar()
{
	if (bBossHealthBarVisible)
		return;

	UWindlessOverlay* overlay = ResolveOverlay();
	if (!overlay)
		return;

	bBossHealthBarVisible = true;
	overlay->ShowBossHealthBar(bossName, attributes ? attributes->GetHealthPercent() : 1.f);

	UE_LOG(LogWindless, Log, TEXT("[보스] 조우: %s"), *bossName.ToString());
}

void ABoss::HideBossHealthBar()
{
	if (!bBossHealthBarVisible)
		return;

	bBossHealthBarVisible = false;

	if (UWindlessOverlay* overlay = ResolveOverlay())
		overlay->HideBossHealthBar();
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();
	HideHealthBar();
}

void ABoss::ShowHealthBar()
{

}

void ABoss::Die_Implementation()
{
	const bool bAlreadyDead = (enemyState == EEnemyState::EES_Dead);

	Super::Die_Implementation();
	if (bAlreadyDead)
		return;

	HideBossHealthBar();
	UE_LOG(LogWindless, Warning, TEXT("%s 보스 사망"), *GetName());

	OnBossDeathBP();
	onBossDeath.Broadcast(this);
}

UWindlessOverlay* ABoss::ResolveOverlay() const
{
	APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);
	if (!controller)
		return nullptr;

	AWindlessHUD* hud = Cast<AWindlessHUD>(controller->GetHUD());
	return hud ? hud->GetWindlessOverlay() : nullptr;
}