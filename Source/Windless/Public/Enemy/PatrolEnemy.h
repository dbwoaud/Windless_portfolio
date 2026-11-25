#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "PatrolEnemy.generated.h"

UCLASS()
class WINDLESS_API APatrolEnemy : public AEnemy
{
	GENERATED_BODY()

public:
	APatrolEnemy();
};