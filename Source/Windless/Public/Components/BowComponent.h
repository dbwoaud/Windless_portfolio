#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BowComponent.generated.h"

class AWindlessCharacter;
class ABow;
class AWeapon;
class UAnimMontage;
class UAnimInstance;
class UUserWidget;
enum class EAttackHand : uint8;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UBowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBowComponent();

	/* 활(입력) */
	void HandleBowPressed(ABow* bow, EAttackHand inputHand); // 공격 버튼 입력 시 실행되는 함수
	void HandleBowReleased(EAttackHand inputHand); // 공격 버튼 입력 해제 시 실행되는 함수
	void CancelDraw(const TCHAR* reason); // 당기기를 취소하는 함수
	void OnWeaponRemoved(const AWeapon* removedWeapon); // 무기 교체 시 실행되는 함수


	/* 활(차징) */
	UFUNCTION(BlueprintPure, Category = "Bow")
	float GetChargeRatio() const; // 현재 차징 비율을 반환하는 함수

protected:
	virtual void BeginPlay() override;


	/* 활(조준) */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Aim")
	TSubclassOf<UUserWidget> crosshairWidgetClass; // 화면 중앙 에임 위젯

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Aim", meta = (ClampMin = "100.0"))
	float aimTraceDistance = 10000.f; // 조준 추적 거리

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Aim")
	TEnumAsByte<ECollisionChannel> aimTraceChannel = ECC_Visibility; // 조준 추적 채널


	/* 활(애니메이션) */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Animation")
	TObjectPtr<UAnimMontage> bowShotMontage; // 활 애니메이션

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Animation")
	FName drawSectionName = FName("Draw"); // 당기기 시작 섹션 이름

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Animation")
	FName fireSectionName = FName("Fire"); // 발사 섹션 이름

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Animation", meta = (ClampMin = "0.0"))
	float cancelBlendOutTime = 0.2f; // 취소 시 몽타주 블렌드 아웃 시간

private:
	/* 캐시 변수 */
	UPROPERTY()
	TObjectPtr<AWindlessCharacter> character; // 캐릭터

	UPROPERTY()
	TObjectPtr<ABow> drawingBow; // 활

	UPROPERTY()
	TObjectPtr<UUserWidget> crosshairWidget; // 에임 위젯


	/* 상태 확인 */
	bool bDrawing = false; // 활을 당기는 중인지 여부
	bool CanStartDraw() const; // 차징을 시작할 수 있는 상태인지 확인하는 함수
	int32 GetAmmoCount(const ABow* bow) const; // 보유 화살 수를 반환하는 함수


	/* 활(차징) */
	EAttackHand drawButton{}; // 차징 버튼
	float drawStartTime = 0.f; // 차징 시작 시각
	void EndDraw(); // 차징을 끝내는 함수


	/* 활(조준) */
	FVector ComputeAimPoint() const; // 조준 지점을 계산하는 함수


	/* 활(공격) */
	void FireArrow(float chargeRatio); // 화살을 발사하는 함수
	bool ConsumeAmmo(const ABow* bow); // 화살 1개를 소모하는 함수

	/* 활(애니메이션) */
	UAnimInstance* GetAnimInstance() const; // 캐릭터 애님 인스턴스를 반환하는 함수


	/* 연출 */
	void SetCrosshairVisible(bool bVisible); // 에임 위젯 표시를 전환하는 함수


public:
	/* 변수 Getter 함수 */
	FORCEINLINE bool IsDrawing() const { return bDrawing; }
};