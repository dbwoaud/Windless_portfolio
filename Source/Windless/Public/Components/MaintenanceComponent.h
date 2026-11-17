#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MaintenanceComponent.generated.h"

class ASavePoint;
class AWindlessCharacter;
class APlayerController;
class UAnimInstance;
class UAnimMontage;
class USavePointMenuWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaintenanceStateChanged, bool, bInMaintenance);

enum class EMaintenanceState : uint8
{
	Idle,    // 일반 플레이
	Active,  // 정비 모드
	Exiting  // 일어서는 중
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UMaintenanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMaintenanceComponent();


	/* 정비 모드 설정 */
	UFUNCTION(BlueprintCallable, Category = "Maintenance")
	bool EnterMaintenance(ASavePoint* savePoint); // 정비 모드에 진입하는 함수

	UFUNCTION(BlueprintCallable, Category = "Maintenance")
	void ExitMaintenance(); // 정비 모드를 종료하는 함수

	UFUNCTION(BlueprintPure, Category = "Maintenance")
	FName GetCurrentSavePointId() const; // 정비 중인 세이브포인트 ID를 반환하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Maintenance|Event")
	FOnMaintenanceStateChanged onMaintenanceStateChanged; // 정비 모드 진입/종료 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	/* 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "Maintenance")
	TSubclassOf<USavePointMenuWidget> menuWidgetClass; // 세이브포인트 메뉴 위젯 클래스

	UPROPERTY(EditDefaultsOnly, Category = "Maintenance")
	int32 menuZOrder = 20; // 메뉴 위젯 Z 순서

	UPROPERTY(EditDefaultsOnly, Category = "Maintenance|Animation")
	TObjectPtr<UAnimMontage> restMontage; // 앉기 애니메이션

	UPROPERTY(EditDefaultsOnly, Category = "Maintenance|Animation")
	FName standUpSection = FName(TEXT("StandUp")); // 일어서기 섹션 이름

private:
	/* 메뉴 설정 */
	void OpenMenu();  // 메뉴를 생성하는 함수
	void CloseMenu(); // 메뉴를 닫는 함수
	void SetMenuInputMode(bool bMenu); // UI/게임 입력 모드를 전환하는 함수

	UFUNCTION()
	void HandleMenuCloseRequested(); // 메뉴에서 종료 요청 시 실행되는 함수

	UFUNCTION()
	void HandleFastTravelStateChanged(bool bTraveling); // 빠른 이동 상태 변경을 처리하는 함수


	/* 애니메이션 */
	void PlayRestMontage();   // 앉기 애니메이션을 재생하는 함수
	bool PlayStandUpSection(); // 일어서기를 재생하고 재생 여부를 반환하는 함수
	void HandleRestMontageEnded(UAnimMontage* montage, bool bInterrupted); // 몽타주 종료 시 실행되는 함수
	void FinishExit(); // 일어서기 완료 후 조작을 복구하는 함수


	/* 사망 */
	UFUNCTION()
	void HandleOwnerDeath(); // 정비 중 사망 시 강제 종료하는 함수


	/* 유틸 */
	AWindlessCharacter* GetOwnerCharacter() const;
	APlayerController* GetOwnerController() const;
	UAnimInstance* GetOwnerAnimInstance() const;
	int32 GetCurrencyAmount() const;

	UPROPERTY()
	TObjectPtr<USavePointMenuWidget> menuWidget = nullptr; // 메뉴 위젯 인스턴스

	UPROPERTY()
	TObjectPtr<ASavePoint> currentSavePoint = nullptr; // 정비 중인 세이브포인트

	EMaintenanceState state = EMaintenanceState::Idle; // 현재 정비 상태

public:
	/* 변수 Getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Maintenance")
	bool IsInMaintenance() const { return state != EMaintenanceState::Idle; }
};