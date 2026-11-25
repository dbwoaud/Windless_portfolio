#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Items/ItemTypes.h"
#include "ItemDatabaseSubsystem.generated.h"

UCLASS()
class WINDLESS_API UItemDatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	/* 조회 */
	UFUNCTION(BlueprintPure, Category = "Item|Database")
	bool GetItemData(FName itemId, FItemData& outData) const; // 아이템 원본을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Item|Database")
	bool HasItem(FName itemId) const; // 아이템 원본이 존재하는지 확인하는 함수


	/* 등록/갱신 */
	void RegisterItem(const FItemData& data); // 아이템 원본을 등록하는 함수
	void Reload(); // 데이터 테이블을 다시 읽어들이는 함수


	/* 헬퍼 */
	static UItemDatabaseSubsystem* Get(const UObject* worldContext); // 월드 컨텍스트로 서브시스템을 반환하는 함수

private:
	/* 초기화 */
	void BuildDatabase(); // 설정된 데이터 테이블에서 원본 캐시를 구성하는 함수


	/* 캐시 */
	TMap<FName, FItemData> itemDatabase; // 아이템 원본 캐시

public:
	/* 변수 getter 함수 */
	FORCEINLINE const TMap<FName, FItemData>& GetAllItems() const { return itemDatabase; }
};