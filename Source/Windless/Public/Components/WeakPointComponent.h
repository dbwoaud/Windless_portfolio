#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/WeakPointRoll.h"
#include "Characters/CombatTypes.h"
#include "WeakPointComponent.generated.h"

class USkeletalMeshComponent;
class UWidgetComponent;
class UWeakPointMarkerWidget;

/* 약점 UI 표시 단계 */
enum class EWeakPointDisplay : uint8
{
	EWD_Hidden,  // 표시 없음
	EWD_Warning, // 각성 1단계: 약점 개수 눈금
	EWD_Detail   // 각성 2,3단계: 약점 위치 상세
};

/* 약점 판정 결과 */
struct FWeakPointHitResult
{
	bool bCritical = false;        // 약점 적중 여부
	FName regionName = NAME_None;  // 맞은 부위 이름
	float awakeningBonus = 1.f;    // 공격자 각성 보너스 배율
	float multiplier = 1.f;        // 최종 대미지 배율
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UWeakPointComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeakPointComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* 약점(판정) */
	FWeakPointHitResult EvaluateHit(const FCombatHitInfo& hitInfo) const; // 타격이 약점에 적중했는지 판정하는 함수
	void RollWeakPoints(); // 약점 부위를 지정하는 함수
	void ApplyRule(const FWeakPointRule& rule); // 원형 약점 규칙을 적용하는 함수


	/* 약점(조회) */
	UFUNCTION(BlueprintPure, Category = "WeakPoint")
	TArray<FName> GetActiveRegionNames() const; // 현재 약점 부위 이름 목록을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "WeakPoint")
	TArray<FName> GetActiveWeakBones() const; // 현재 약점 부위의 본 목록을 반환하는 함수

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/* 캐시 변수 */
	TArray<int32> activeRegionIndices; // 활성화된 약점 목록
	TMap<FName, int32> boneToRegionMap; // 본 이름과 약점 후보 목록


	/* 컴포넌트 */
	TWeakObjectPtr<UAttributeComponent> playerAttributes; // 플레이어 속성 컴포넌트


	/* 약점(규칙) */
	FWeakPointRule BuildComponentRule() const; // 컴포넌트 설정으로 약점 규칙을 만드는 함수
	FWeakPointRule overrideRule; // 원형에서 받은 약점 규칙
	bool bHasOverrideRule = false; // 원형 규칙 사용 여부


	/* 약점(설정) */
	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint")
	TArray<FWeakPointRegion> candidateRegions; // 약점 후보 부위 목록

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint")
	bool bFixedWeakPoints = false; // 고정 약점 사용 여부

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint", meta = (EditCondition = "bFixedWeakPoints"))
	TArray<FName> fixedRegionNames; // 고정 약점 부위 이름 목록

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint", meta = (ClampMin = "0", EditCondition = "!bFixedWeakPoints"))
	int32 minWeakPointCount = 1; // 무작위 약점 최소 개수

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint", meta = (ClampMin = "0", EditCondition = "!bFixedWeakPoints"))
	int32 maxWeakPointCount = 2; // 무작위 약점 최대 개수

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint", meta = (ClampMin = "0.0"))
	float fallbackSearchRadius = 50.f; // 본 정보가 없을 때 가장 가까운 본을 찾는 최대 거리


	/* 약점(UI) */
	void CreateMarkers(); // 현재 약점에 맞춰 마커 위젯 컴포넌트를 생성하는 함수

	// 마커 컴포넌트를 생성하는 함수
	UWidgetComponent* CreateMarkerComponent(TSubclassOf<UWeakPointMarkerWidget> widgetClass, USceneComponent* parent, FName socketName, const FVector& offset); 

	void UpdateMarkerVisibility(); // 마커 표시를 갱신하는 함수
	bool IsMarkerOccluded(const FVector& viewLocation, const FVector& markerLocation) const; // 카메라에서 마커까지 시야가 막혔는지 확인하는 함수
	static EWeakPointDisplay GetDisplayForChaos(EChaosState state); // 혼돈 단계를 약점 표시 단계로 변환하는 함수

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint|UI")
	TSubclassOf<UWeakPointMarkerWidget> warningMarkerClass; // 각성 1단계 경고 마커 위젯

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint|UI")
	TSubclassOf<UWeakPointMarkerWidget> detailMarkerClass; // 각성 2단계 이상 부위 마커 위젯

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint|UI")
	FVector warningMarkerOffset = FVector(0.f, 0.f, 120.f); // 경고 마커 위치

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint|UI", meta = (ClampMin = "0.0"))
	float markerVisibleDistance = 2500.f; // 마커 표시 최대 거리

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint|UI", meta = (ClampMin = "0.05"))
	float markerUpdateInterval = 0.25f; // 거리/사망 체크 주기

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint|UI")
	bool bHideWhenOccluded = true; // 카메라와 마커 사이가 지형에 가려지면 숨길지 여부

	UPROPERTY(EditDefaultsOnly, Category = "WeakPoint|UI")
	TEnumAsByte<ECollisionChannel> occlusionTraceChannel = ECC_Visibility; // 시야 차단 판정 채널

	UPROPERTY()
	TObjectPtr<UWidgetComponent> warningMarker; // 경고 마커

	UPROPERTY()
	TArray<TObjectPtr<UWidgetComponent>> detailMarkers; // 부위별 상세 마커

	EWeakPointDisplay currentDisplay = EWeakPointDisplay::EWD_Hidden; // 현재 표시 단계
	FTimerHandle markerUpdateTimer; // 마커 갱신 타이머


	/* 약점(디버그용) */
	UPROPERTY(EditAnywhere, Category = "WeakPoint|Debug")
	bool bDrawDebug = false; // 약점 본 위치 디버그 표시 여부

	UPROPERTY(EditAnywhere, Category = "WeakPoint|Debug", meta = (ClampMin = "1.0"))
	float debugSphereRadius = 12.f; // 디버그 구 반지름


	/* 혼돈 시스템 */
	void BindPlayerChaos(); // 플레이어 혼돈 단계 변경을 구독하는 함수

	UFUNCTION()
	void HandlePlayerChaosChanged(EChaosState newState); // 플레이어 혼돈 단계 변경 시 실행되는 함수


	/* 헬퍼 함수 */
	void BuildBoneMap(); // 본 맵을 구성하는 함수
	int32 FindRegionIndex(const USkeletalMeshComponent* mesh, FName hitBone, const FVector& impactPoint) const; // 피격 본이 속한 후보 인덱스를 찾는 함수
	int32 FindClosestRegionIndex(const USkeletalMeshComponent* mesh, const FVector& impactPoint) const; // 타격 지점과 가장 가까운 후보 인덱스를 찾는 함수
	float GetAttackerAwakeningBonus(const AActor* attacker) const; // 공격자의 각성 약점 보너스를 반환하는 함수
	USkeletalMeshComponent* GetOwnerMesh() const; // 소유 캐릭터의 메시를 반환하는 함수


	/* 디버그용 */
	void DrawWeakPointDebug() const; // 약점 본 위치를 디버그 구로 그리는 함수
};