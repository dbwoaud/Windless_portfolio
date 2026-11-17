#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

class AItems;
class ASoul;
class ATreasure;

UINTERFACE(MinimalAPI)
class UPickupInterface : public UInterface
{
	GENERATED_BODY()
};

// 줍기 가능한 액터를 정의하는 인터페이스 
class WINDLESS_API IPickupInterface
{
	GENERATED_BODY()

public:
	virtual void SetOverlappingItem(AItems* item); // overlap된 아이템을 설정하는 함수
	virtual void AddSouls(ASoul* soul); // 아이템을 주울 시 소울을 획득하는 함수
	virtual void AddGold(ATreasure* treasure); // 아이템을 주울 시 골드를 획득하는 함수
};