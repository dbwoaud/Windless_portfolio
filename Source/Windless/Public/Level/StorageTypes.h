#pragma once

#include "CoreMinimal.h"
#include "Save/WindlessSaveTypes.h"
#include "StorageTypes.generated.h"

// 창고 등급
UENUM(BlueprintType)
enum class EStorageGrade : uint8
{
	ESG_Small  UMETA(DisplayName = "Small"),  // 소형(10x30)
	ESG_Medium UMETA(DisplayName = "Medium"), // 중형(10x45)
	ESG_Large  UMETA(DisplayName = "Large"),  // 대형(10x60)
	ESG_Huge   UMETA(DisplayName = "Huge"),   // 특대형(10x75)
	ESG_MAX    UMETA(Hidden)
};

/* 창고 등급별 세로 칸수를 반환하는 함수 */
inline int32 GetStorageGradeHeight(EStorageGrade grade)
{
	switch (grade)
	{
	case EStorageGrade::ESG_Medium: return 45;
	case EStorageGrade::ESG_Large:  return 60;
	case EStorageGrade::ESG_Huge:   return 75;
	default:                        return 30;
	}
}

/* 창고 등급 이름을 반환하는 함수 */
inline FText GetStorageGradeName(EStorageGrade grade)
{
	switch (grade)
	{
	case EStorageGrade::ESG_Medium: return FText::FromString(TEXT("중형"));
	case EStorageGrade::ESG_Large:  return FText::FromString(TEXT("대형"));
	case EStorageGrade::ESG_Huge:   return FText::FromString(TEXT("특대형"));
	default:                        return FText::FromString(TEXT("소형"));
	}
}

static constexpr int32 StorageGridWidth = 10; // 창고 가로 칸수

/* 통합 창고 저장 레코드 */
USTRUCT()
struct FStorageSaveRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FInventorySaveRecord inventory; // 창고 내용

	UPROPERTY(SaveGame)
	EStorageGrade grade = EStorageGrade::ESG_Small; // 창고 등급
};
