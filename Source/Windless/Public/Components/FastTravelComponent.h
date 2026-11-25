#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Save/WindlessSaveTypes.h"
#include "FastTravelComponent.generated.h"

class AWindlessCharacter;
class APlayerController;

/* 빠른 이동 차단 사유 */
UENUM(BlueprintType)
enum class EFastTravelBlockReason : uint8
{
	EFTB_None        UMETA(DisplayName = "가능"),
	EFTB_InCombat    UMETA(DisplayName = "전투 중"),
	EFTB_NotAtPoint  UMETA(DisplayName = "세이브포인트 아님"),
	EFTB_SamePoint   UMETA(DisplayName = "현재 위치"),
	EFTB_NotActive   UMETA(DisplayName = "미활성 목적지"),
	EFTB_Busy        UMETA(DisplayName = "이동 중")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFastTravelStateChanged, bool, bTraveling);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UFastTravelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFastTravelComponent();

	/* 빠른 이동*/
	UFUNCTION(BlueprintCallable, Category = "FastTravel")
	bool TravelTo(FName destinationId); // 목적지로 이동을 시작하는 함수

	UFUNCTION(BlueprintPure, Category = "FastTravel")
	EFastTravelBlockReason GetBlockReason(FName destinationId) const; // 이동 차단 사유를 반환하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "FastTravel|Event")
	FOnFastTravelStateChanged onFastTravelStateChanged; // 이동 시작/종료 시 실행되는 델리게이트

protected:
	/* 연출 설정 */
	UPROPERTY(EditDefaultsOnly, Category = "FastTravel|Fade", meta = (ClampMin = "0.0"))
	float fadeOutDuration = 0.6f; // 페이드 아웃 시간

	UPROPERTY(EditDefaultsOnly, Category = "FastTravel|Fade", meta = (ClampMin = "0.0"))
	float loadingDuration = 1.2f; // 로딩 시간

	UPROPERTY(EditDefaultsOnly, Category = "FastTravel|Fade", meta = (ClampMin = "0.0"))
	float fadeInDuration = 0.8f; // 페이드 인 시간

	UPROPERTY(EditDefaultsOnly, Category = "FastTravel|Fade")
	FLinearColor fadeColor = FLinearColor::Black; // 페이드 색

private:
	/* 캐시 변수 */
	AWindlessCharacter* GetOwnerCharacter() const;
	APlayerController* GetOwnerController() const;


	/* 빠른 이동 */
	void PerformTeleport(); // 페이드 후 실제 좌표 이동을 수행하는 함수
	void FinishTravel(); // 페이드 인과 상태 복구를 수행하는 함수
	
	FSavePointRecord pendingDestination; // 이동 목적지 레코드
	FTimerHandle travelTimer; // 연출 타이머
	bool bTraveling = false; // 빠른 이동 진행 여부

public:
	/* 변수 Getter 함수 */
	UFUNCTION(BlueprintPure, Category = "FastTravel")
	FORCEINLINE bool IsTraveling() const { return bTraveling; }
};