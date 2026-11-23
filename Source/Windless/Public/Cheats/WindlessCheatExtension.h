#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "WindlessCheatExtension.generated.h"

class AWindlessCharacter;

/*
 * 콘솔 치트의 공통 부모 클래스
 */

UCLASS(Abstract)
class WINDLESS_API UWindlessCheatExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	UWindlessCheatExtension();

protected:
	/* 캐시 변수 */
	APawn* GetPlayerPawn() const; // 플레이어 폰을 반환하는 함수
	AWindlessCharacter* GetPlayerCharacter() const; // 플레이어 캐릭터를 반환하는 함수

	template <typename TComponent>
	TComponent* GetPlayerComponent() const // 플레이어의 컴포넌트를 반환하는 함수
	{
		const APawn* pawn = GetPlayerPawn();
		return pawn ? pawn->FindComponentByClass<TComponent>() : nullptr;
	}
};