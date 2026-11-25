#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Characters/CombatTypes.h"
#include "CombatComponent.generated.h"

class UEnhancedInputComponent;
class UWindlessInputConfig;
class AWindlessCharacter;
class AWeapon;
class UAnimMontage;

enum class EAttackHand : uint8
{
	Left,
	Right
};

/* 공격 입력 요청 */
struct FAttackInputRequest
{
	bool bValid = false; // 공격 입력 요청 유효 여부
	EAttackHand hand = EAttackHand::Right; // 공격 손
	EAttackType attackType = EAttackType::EAT_Light; // 공격 종류

	void Reset() { bValid = false; }
};

USTRUCT(BlueprintType)
struct FAttackProfile
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UAnimMontage* normalMontage = nullptr; // 약공격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UAnimMontage* strongMontage = nullptr; // 강공격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UAnimMontage* jumpMontage = nullptr; // 점프 공격 몽타주
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();


	/* 전투 시스템(입력) */
	void BindCombatInputs(UEnhancedInputComponent* enhancedInput, const UWindlessInputConfig& input); // 공격 관련 입력을 컴포넌트에 바인딩하는 함수
	void HandleAttackPressed(EAttackHand hand, EAttackType attackType); // 공격 버튼을 누른 순간을 처리하는 함수
	void HandleAttackHeld(EAttackHand hand); // 공격 버튼 유지를 처리하는 함수
	void HandleAttackReleased(EAttackHand hand); // 공격 버튼 해제를 처리하는 함수


	/* 전투 시스템(점프 공격) */
	void OnOwnerLanded(); // 캐릭터 착지 시 실행되는 함수


	/* 전투 시스템(콤보) */
	void OpenComboWindow(); // 콤보 입력 구간을 여는 함수
	void CloseComboWindow(); // 콤보 입력 구간을 닫고 예약된 다음 타를 실행하는 함수
	void ResetCombo(); // 콤보 상태를 초기화하는 함수


	/* 전투 시스템(방어) */
	void StopGuard(); // 가드를 해제하는 함수
	bool IsGuardingAgainst(AActor* hitter) const; // 정면 가드 판정을 확인하는 함수
	bool HandleGuard(AActor* attacker, const FCombatHitInfo& hitInfo, FGuardResult& outResult); // 가드를 처리하는 함수


	/* 전투 시스템(무기 교체) */
	void OnHandWeaponChanged(AWeapon* removedWeapon); // 무기 변경 시 실행되는 함수


	/* 상태 확인 */
	bool IsGuardOnLeftHand() const; // 현재 왼쪽 팔로 가드하고 있는지 확인하는 함수

protected:
	virtual void BeginPlay() override;

private:
	/* 컴포넌트 */
	UPROPERTY()
	AWindlessCharacter* character = nullptr; // 소유 캐릭터


	/* 전투 시스템(입력) */
	void ResolvePendingAttack(); // 같은 프레임에 들어온 입력을 처리하는 함수
	EAttackHand ResolveAttackHand(EAttackHand hand) const; // 양손무기 장착 시 좌/우 입력을 하나로 통합하는 함수

	FAttackInputRequest pendingInput; // 이번 프레임에 확정 대기 중인 입력


	/* 전투 시스템(공격) */
	void ExecuteAttack(EAttackHand hand, EAttackType attackType); // 좌/우, 약/강 공격을 통합 처리하는 함수
	AWeapon* GetHandWeapon(EAttackHand hand) const; // 손에 든 무기를 반환하는 함수
	const FAttackProfile& GetAttackProfile(EAttackHand hand, const AWeapon* weapon) const; // 무기 타입/손에 맞는 애니메이션을 반환하는 함수
	UAnimMontage* GetAttackMontage(const FAttackProfile& profile, EAttackType attackType) const; // 공격 종류에 맞는 애니메이션을 반환하는 함수
	UAnimMontage* GetShieldJumpMontage(EAttackHand hand) const; // 방패 든 손에 맞는 점프 공격 몽타주를 반환하는 함수

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	FAttackProfile leftHandAttacks;  // 왼손 약/강공격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	FAttackProfile rightHandAttacks; // 오른손 약/강공격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	FAttackProfile twoHandedAttacks; // 양손무기 약/강공격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	TObjectPtr<UAnimMontage> leftShieldJumpMontage; // 왼손 방패 점프 공격 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	TObjectPtr<UAnimMontage> rightShieldJumpMontage; // 오른손 방패 점프 공격 몽타주


	/* 전투 시스템(점프 공격) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|JumpAttack")
	FName jumpLandSectionName = FName("Land"); // 점프 공격 애니메이션의 착지 섹션 이름

	bool bJumpAttackUsed = false; // 점프 공격 사용 여부


	/* 전투 시스템(콤보) */
	bool TryAdvanceCombo(); // 다음 콤보로 연결을 시도하는 함수

	UPROPERTY(EditAnywhere, Category = "Combat|Combo", meta = (ClampMin = "1", ClampMax = "3"))
	int32 maxComboCount = 3; // 최대 콤보

	FAttackInputRequest activeCombo; // 진행 중인 콤보의 손/종류
	int32 comboIndex = 0; // 현재 콤보 인덱스
	bool bComboWindowOpen = false; // 콤보 입력 구간이 열려 있는지 여부
	bool bComboInputBuffered = false; // 구간 내 다음 콤보 입력 여부

	UPROPERTY()
	UAnimMontage* comboMontage = nullptr; // 콤보 몽타주

	UPROPERTY()
	AWeapon* comboWeapon = nullptr; // 콤보 무기



	/* 전투 시스템(방어) */
	void StartGuardWithHand(AWeapon* handWeapon); // 방패로 가드를 시작하는 함수

	UPROPERTY()
	AWeapon* guardingWeapon = nullptr; // 캐릭터가 소유한 방패 무기

	bool bIsGuarding = false; // 가드 유지 중인지 확인하는 변수


	/* 상태 확인 */
	bool IsAttackArmDisabled(EAttackHand hand, const AWeapon* weapon) const; // 공격에 필요한 팔 부위가 파괴되었는지 확인하는 함수
	bool IsOwnerInAir() const; // 캐릭터가 공중에 있는지 확인하는 함수

public:
	/* 변수 getter 함수 */
	FORCEINLINE bool IsGuarding() const { return bIsGuarding; }
};