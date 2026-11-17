#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingComponent.generated.h"

class UWidgetComponent;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AActor*, NewTarget);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	

	/* 락온 */
	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void ToggleLockOn(); // 락온을 토글하는 함수

	void ReleaseLockOn(const TCHAR* reason); // 락온을 강제 해제하는 함수

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	void SwitchTarget(float Direction); // 락온 타겟을 좌우 방향으로 전환하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Targeting")
	FOnTargetChanged OnTargetChanged; // 락온 타겟 변경 시 호출되는 델리게이트

protected:
	virtual void BeginPlay() override;

	/* 락온 (적 탐색)*/
	UPROPERTY(EditAnywhere, Category = "Targeting|Search")
	float MaxLockOnDistance = 1500.f; // 락온 가능 최대 거리

	UPROPERTY(EditAnywhere, Category = "Targeting|Search")
	float BreakLockDistance = 2000.f; // 락온 자동 해제 거리

	UPROPERTY(EditAnywhere, Category = "Targeting|Search")
	float SearchSphereRadius = 1500.f; // 타겟 탐색 시 overlap의 반경

	UPROPERTY(EditAnywhere, Category = "Targeting|Search")
	FName RequiredTag = FName("Enemy"); // 락온 타겟이 가져야 하는 태그

	UPROPERTY(EditAnywhere, Category = "Targeting|Search")
	FName DeadTag = FName("Dead"); // 사망한 액터를 식별하는 태그

	UPROPERTY(EditAnywhere, Category = "Targeting|Search") 
	TArray<TEnumAsByte<EObjectTypeQuery>> TargetObjectTypes; // 타겟 후보 탐색 시 검사할 오브젝트 타입 

	UPROPERTY(EditAnywhere, Category = "Targeting|Search")
	TEnumAsByte<ETraceTypeQuery> VisibilityTraceChannel; // 시야 확보 라인 추적에 사용되는 채널
	
	UPROPERTY(EditAnywhere, Category = "Targeting|Search", meta = (ClampMin = "0.0"))
	float lineOfSightGraceTime = 0.5f; // 시야가 가려져도 락온을 유지하는 유예 시간


	/* 락온(적 선택) */
	UPROPERTY(EditAnywhere, Category = "Targeting|Selection", meta = (ClampMin = "0.0", ClampMax = "1.5"))
	float MaxScreenDistance = 0.6f;  // 화면 중심에서 후보까지의 최대 허용 거리

	UPROPERTY(EditAnywhere, Category = "Targeting|Selection")
	float ScreenCenterWeight = 2.f; // 베스트 타겟 선정 시 화면 중앙 근접도 가중치

	UPROPERTY(EditAnywhere, Category = "Targeting|Selection")
	float DistanceWeight = 1.f; // 베스트 타겟 선정 시 월드 거리 근접도 가중치


	/* 락온 (적 전환) */
	UPROPERTY(EditAnywhere, Category = "Targeting|Switching")
	float SwitchCoolTime = 0.25f; // 타겟 전환 입력 쿨다운


	/* 락온 (카메라 설정) */
	UPROPERTY(EditAnywhere, Category = "Targeting|Camera")
	float CameraInterpSpeed = 6.f; // 락온 중 카메라가 타겟을 향해 회전하는 보간 속도

	UPROPERTY(EditAnywhere, Category = "Targeting|Camera")
	float TargetPitchOffset = -15.f; // 락온 시 카메라 피치 오프셋


	/* 락온 마커 */
	UPROPERTY(EditAnywhere, Category = "Targeting|Marker")
	TSubclassOf<UUserWidget> lockOnMarkerClass; // 락온 대상에 표시할 마커 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "Targeting|Marker")
	FVector2D lockOnMarkerSize = FVector2D(16.f, 16.f); // 마커 위젯의 화면 크기

	UPROPERTY(EditAnywhere, Category = "Targeting|Marker")
	FVector lockOnMarkerOffset = FVector(0.f, 0.f, 20.f); // 대상 루트 기준 마커 위치 오프셋


private:
	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<class ACharacter> WindlessCharacter; // 소유 캐릭터 

	UPROPERTY()
	TObjectPtr<class APlayerController> PlayerController; // 소유 캐릭터의 플레이어 컨트롤러

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget; // 현재 락온된 대상

	bool bLockActive = false; // 락온 진행 여부
	float lostSightTime = 0.f; // 대상이 시야에서 가려진 누적 시간


	/* 락온 */
	void FindAndLockTarget(); // 최적 타겟을 탐색하여 락온하는 함수
	void SetLockTarget(AActor* newTarget); // 락온 대상을 지정하는 함수
	void BreakLock(const TCHAR* reason); // 락온을 해제하는 함수


	/* 락온(적 탐색) */
	void CollectCandidates(TArray<AActor*>& OutCandidates) const; // 락온 후보 액터를 수집하는 함수
	bool IsCandidateEligible(AActor* Candidate) const; // 후보가 락온 가능 조건을 만족하는지 확인하는 함수
	bool IsValidTargetActor(const AActor* candidate) const; // 대상이 살아있는 적인지 확인하는 함수
	bool IsWithinRange(const AActor* candidate, float baseRange) const; // 부상 배율을 적용한 거리 안에 있는지 확인하는 함수
	bool HasLineOfSight(const AActor* Target) const; // 대상까지 시야가 차단되지 않았는지 확인하는 함수
	bool GetScreenPosition(const AActor* Target, FVector2D& OutScreenPos) const; // 대상의 월드 위치를 스크린 좌표로 투영하는 함수


	/* 락온(전 선택) */
	AActor* FindBestTarget(const TArray<AActor*>& Candidates) const; // 락온 후보 중 최적 타겟을 선정하는 함수
	FVector GetTargetingLocation(const AActor* Target) const; // 대상의 락온 기준 위치를 반환하는 함수


	/* 락온(적 전환) */
	AActor* FindSwitchTarget(float Direction) const; // 입력 방향에 위치한 가장 가까운 다음 타겟을 찾는 함수
	float LastSwitchTime = 0.f; // 마지막 타겟 전환 시각
	
	/* 락온(카메라 설정) */
	void UpdateCameraOnTarget(float DeltaTime); // 락온 중 카메라를 대상 방향으로 회전시키는 함수


	/* 락온 마커 */
	UPROPERTY()
	TObjectPtr<UWidgetComponent> lockOnMarker; // 락온 마커 위젯 컴포넌트

	void CreateLockOnMarker(); // 마커를 생성하는 함수
	void ShowLockOnMarker(AActor* target); // 마커를 대상에 부착하고 표시하는 함수
	void HideLockOnMarker(); // 마커를 숨기는 함수

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Targeting")
	FORCEINLINE bool IsLockedOn() const { return CurrentTarget != nullptr; }
	UFUNCTION(BlueprintPure, Category = "Targeting")
	FORCEINLINE AActor* GetCurrentTarget() const { return CurrentTarget; }
};