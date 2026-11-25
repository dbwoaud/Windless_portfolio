#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Boss.generated.h"

class ABoss;
class UWindlessOverlay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossDeathSignature, ABoss*, deadBoss);

UCLASS()
class WINDLESS_API ABoss : public AEnemy
{
	GENERATED_BODY()

public:
	/* 전투 시스템(피격) */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


	/* 보스 체력 바 */
	void ShowBossHealthBar(); // 화면 상단 보스 체력 바를 표시하는 함수
	void HideBossHealthBar(); // 화면 상단 보스 체력 바를 숨기는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Boss|Event")
	FOnBossDeathSignature onBossDeath; // 보스 사망 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;


	/* 보스 설정 */
	UPROPERTY(EditAnywhere, Category = "Boss")
	FText bossName; // 보스 이름


	/* UI */
	virtual void ShowHealthBar() override; // 보스는 머리 위 체력 바를 사용하지 않음


	/* 전투 시스템(사망) */
	virtual void Die_Implementation() override; // 보스 사망 처리 + 델리게이트 브로드캐스트

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Event")
	void OnBossDeathBP(); // 보스 사망 시 실행되는 블루프린트용 함수


	/* 오버레이 */
	UWindlessOverlay* ResolveOverlay() const; // 플레이어 오버레이를 반환하는 함수

	bool bBossHealthBarVisible = false; // 화면 상단 보스 체력 바 표시 여부

public:
	/* 변수 Getter 함수 */
	FORCEINLINE FText GetBossName() const { return bossName; }
};