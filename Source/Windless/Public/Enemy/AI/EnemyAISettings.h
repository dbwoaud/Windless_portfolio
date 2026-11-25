#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EnemyAISettings.generated.h"

class UDataTable;
class USoundAttenuation;

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Enemy AI"))
class WINDLESS_API UEnemyAISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName("Windless"); }


	/* 데이터 테이블 */
	UPROPERTY(config, EditAnywhere, Category = "Database")
	TSoftObjectPtr<UDataTable> archetypeTable; // 몬스터 원형 데이터 테이블

	UPROPERTY(config, EditAnywhere, Category = "Database")
	TSoftObjectPtr<UDataTable> attackTable; // 적 공격 데이터 테이블

	UPROPERTY(config, EditAnywhere, Category = "Database")
	TSoftObjectPtr<UDataTable> cardBaseTable; // 카드 기본 점수 테이블

	UPROPERTY(config, EditAnywhere, Category = "Database")
	TSoftObjectPtr<UDataTable> playerStateModTable; // 플레이어 상태 배율 테이블 

	UPROPERTY(config, EditAnywhere, Category = "Database")
	TSoftObjectPtr<UDataTable> lastActModTable; // 직전 행동 보정 테이블


	/* 플레이어 참조 값 */
	UPROPERTY(config, EditAnywhere, Category = "Reference", meta = (ClampMin = "0.0"))
	float playerReferenceToughness = 15.f; // 플레이어 강인도

	UPROPERTY(config, EditAnywhere, Category = "Reference", meta = (ClampMin = "0.0"))
	float playerWeakStaggerTime = 0.6f; // 플레이어 약한 경직 시간

	UPROPERTY(config, EditAnywhere, Category = "Reference", meta = (ClampMin = "0.0"))
	float playerRollInvincibleTime = 0.35f; // 플레이어 구르기 무적 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Player State", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float frontHalfAngle = 60.f; // 정면 반각

	UPROPERTY(config, EditAnywhere, Category = "Card|Player State", meta = (ClampMin = "0.0"))
	float counterRange = 3.f; // 휘두르기 시작 및 등 뒤 반응 거리

	UPROPERTY(config, EditAnywhere, Category = "Card|Player State", meta = (ClampMin = "0.0"))
	float itemPunishRange = 3.5f; // 회복 응징 거리

	UPROPERTY(config, EditAnywhere, Category = "Card|Player State", meta = (ClampMin = "0.0"))
	float aimRange = 8.f; // 활 조준 반응 거리

	UPROPERTY(config, EditAnywhere, Category = "Card|Player State", meta = (ClampMin = "0.0"))
	float guardHoldThreshold = 2.f; // 가드 유지 기준 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Player State", meta = (ClampMin = "0.0"))
	float signalReselectInterval = 0.5f; // 신호 재선택 주기


	/* 비공격 카드 동작 */
	UPROPERTY(config, EditAnywhere, Category = "Card|Actions", meta = (ClampMin = "0.0"))
	float restTimeMin = 0.5f; // 숨 고르기 최소 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Actions", meta = (ClampMin = "0.0"))
	float restTimeMax = 1.2f; // 숨 고르기 최대 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Actions", meta = (ClampMin = "0.0"))
	float sideStepTimeMin = 1.5f; // 옆걸음 최소 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Actions", meta = (ClampMin = "0.0"))
	float sideStepTimeMax = 2.5f; // 옆걸음 최대 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Actions", meta = (ClampMin = "0.0"))
	float backOffDistance = 2.5f; // 물러서기 거리

	UPROPERTY(config, EditAnywhere, Category = "Card|Actions", meta = (ClampMin = "0.0"))
	float approachMaxTime = 3.f; // 다가가기 최대 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Actions", meta = (ClampMin = "0.0"))
	float rangedBackOffCooldown = 4.f; // 원거리 뒤로 빠지기 쿨타임


	/* 도달 불가 및 노려보기 */ 
	UPROPERTY(config, EditAnywhere, Category = "Card|Reach", meta = (ClampMin = "0.0"))
	float glareTime = 4.f; // 노려보기 시간

	UPROPERTY(config, EditAnywhere, Category = "Card|Reach", meta = (ClampMin = "1.0"))
	float unreachablePathRatio = 3.f; // 도달 불가 거리 비율

	UPROPERTY(config, EditAnywhere, Category = "Card|Reach", meta = (ClampMin = "0.0"))
	float pathCheckHeight = 3.f; // 경로 확인 높이


	static const UEnemyAISettings* Get() { return GetDefault<UEnemyAISettings>(); }


	/* 공격 연출 */
	UPROPERTY(config, EditAnywhere, Category = "Attack")
	TSoftObjectPtr<USoundAttenuation> windupSoundAttenuation; // 예고 효과음 3D 감쇠


	/* 경직 */
	UPROPERTY(config, EditAnywhere, Category = "Stagger", meta = (ClampMin = "0.1"))
	float weakStaggerTime = 0.6f; // 몬스터 약한 경직 시간

	UPROPERTY(config, EditAnywhere, Category = "Stagger", meta = (ClampMin = "0.05"))
	float knockbackTime = 0.5f; // 강한 경직(날아감 시간)

	UPROPERTY(config, EditAnywhere, Category = "Stagger", meta = (ClampMin = "0.05"))
	float downTime = 1.0f; // 강한 경직(다운 시간)

	UPROPERTY(config, EditAnywhere, Category = "Stagger", meta = (ClampMin = "0.05"))
	float getUpTime = 0.8f; // 강한 경직(기상 시간)

	UPROPERTY(config, EditAnywhere, Category = "Stagger", meta = (ClampMin = "0.0"))
	float knockbackDistance = 3.0f; // 강한 경직 날아가는 거리
};