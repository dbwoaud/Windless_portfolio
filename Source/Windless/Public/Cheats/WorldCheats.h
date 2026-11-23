#pragma once

#include "CoreMinimal.h"
#include "Cheats/WindlessCheatExtension.h"
#include "WorldCheats.generated.h"

class UWindlessSaveSubsystem;

/*
 * 세이브, 월드 서브시스템 콘솔 치트
 */

UCLASS()
class WINDLESS_API UWorldCheats : public UWindlessCheatExtension
{
	GENERATED_BODY()

public:
	/* 세이브 */
	UFUNCTION(Exec)
	void WLSave(); // 현재 위치를 기본 세이브 포인트로 저장하는 함수

	UFUNCTION(Exec)
	void WLLoad(); // 저장된 세이브 데이터를 적용하는 함수

	UFUNCTION(Exec)
	void WLDeleteSave(); // 저장된 세이브 데이터를 삭제하는 함수

	UFUNCTION(Exec)
	void WLSaveInfo(); // 저장된 세이브 데이터를 출력하는 함수


	/* 월드 서브시스템 */
	UFUNCTION(Exec)
	void WLSavePoints(); // 세이브 포인트 활성화 상태를 출력하는 함수

	UFUNCTION(Exec)
	void WLStorage(); // 공유 창고 상태를 출력하는 함수

	UFUNCTION(Exec)
	void WLLandmarks(); // 랜드마크 발견 상태를 출력하는 함수

	UFUNCTION(Exec)
	void WLSoulMarker(); // 영혼 마커 상태를 출력하는 함수

private:
	UWindlessSaveSubsystem* GetSaveSubsystem() const; // 세이브 서브시스템을 반환하는 함수
};