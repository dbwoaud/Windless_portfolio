#include "Items/WorldItem.h"
#include "Items/Database/ItemDatabaseSubsystem.h" 
#include "Characters/WindlessCharacter.h"
#include "Components/InventoryComponent.h"
#include "Components/InteractionSphereComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/PlayerInteractionComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Windless/DebugMacro.h"

AWorldItem::AWorldItem()
{
	PrimaryActorTick.bCanEverTick = false;

	itemCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ItemCollision"));
	itemCollision->InitSphereRadius(collisionRadius);
	itemCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	itemCollision->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	itemCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	itemCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	itemCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	RootComponent = itemCollision;

	itemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	itemMesh->SetupAttachment(itemCollision);
	itemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	beamEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeamEffect"));
	beamEffect->SetupAttachment(itemCollision);
	beamEffect->SetUsingAbsoluteRotation(true);
	beamEffect->bAutoActivate = true;

	beamLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeamLight"));
	beamLight->SetupAttachment(itemCollision);
	beamLight->SetUsingAbsoluteRotation(true);
	beamLight->SetMobility(EComponentMobility::Movable);
	beamLight->SetCastShadows(false);
	beamLight->SetRelativeLocation(FVector(0.f, 0.f, 40.f));

	interactionSphere = CreateDefaultSubobject<UInteractionSphereComponent>(TEXT("InteractionSphere"));
	interactionSphere->SetupAttachment(itemCollision);
	interactionSphere->InitSphereRadius(interactionRadius);
	interactionSphere->SetUsingAbsoluteRotation(true);
}

void AWorldItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPresentation();
}

void AWorldItem::Interact_Implementation(AActor* interactor)
{
	AWindlessCharacter* player = Cast<AWindlessCharacter>(interactor);
	if (!player)
		return;

	UInventoryComponent* inventory = player->GetInventory();
	if (!inventory)
		return;

	if (!inventory->IsItemRegistered(itemId))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Magenta,
				FString::Printf(TEXT("[월드 아이템] 인벤토리에 등록되지 않은 아이템: %s"), *itemId.ToString()));
		}

		UE_LOG(LogWindless, Warning,
			TEXT("[월드 아이템] 원본 미등록: %s / InventoryComponent의 itemDataTables에 해당 행이 있는지 확인 필요"),
			*itemId.ToString());
		return;
	}

	FItemInstance pickup;
	pickup.itemId = itemId;
	pickup.currentStackCount = FMath::Max(1, quantity);
	pickup.currentDurability = durability;

	int32 addedCount = 0;
	const bool bAll = inventory->AddItemInstance(pickup, addedCount);

	if (addedCount <= 0)
	{
		if (GEngine && !inventory->IsCarryLimitReached(itemId))
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("[인벤토리] 공간이 부족합니다"));

		UE_LOG(LogWindless, Log, TEXT("[월드 아이템] 획득 실패: %s (한도 도달 %s)"),
			*itemId.ToString(), inventory->IsCarryLimitReached(itemId) ? TEXT("O") : TEXT("X"));
		return;
	}

	PlayPickupEffect();

	if (!bAll)
	{
		quantity -= addedCount;
		OnPickedUpBP();
		UE_LOG(LogWindless, Log, TEXT("[월드 아이템] 부분 획득: %s %d개 획득 / %d개 잔류"),
			*itemId.ToString(), addedCount, quantity);

		return;
	}

	UE_LOG(LogWindless, Log, TEXT("[월드 아이템] 획득 완료: %s x%d"), *itemId.ToString(), addedCount);

	OnPickedUpBP();
	if (UPlayerInteractionComponent* interaction = player->FindComponentByClass<UPlayerInteractionComponent>())
		interaction->UnregisterCandidate(this);

	Destroy();
}

FText AWorldItem::GetInteractionText_Implementation()
{
	FItemData data;
	const FText displayName = LookupItemData(data) ? data.itemName : FText::FromName(itemId);

	if (const AWindlessCharacter* player = Cast<AWindlessCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		if (const UInventoryComponent* inventory = player->GetInventory())
		{
			if (inventory->IsCarryLimitReached(itemId))
				return FText::FromString(FString::Printf(TEXT("%s  (소지 한도 %d개 도달)"), *displayName.ToString(), inventory->GetCarryLimit(itemId)));
		}
	}

	if (quantity > 1)
		return FText::FromString(FString::Printf(TEXT("%s  %s x%d"), *interactionText.ToString(), *displayName.ToString(), quantity));

	return FText::FromString(FString::Printf(TEXT("%s  %s"), *interactionText.ToString(), *displayName.ToString()));
}

bool AWorldItem::CanInteract_Implementation(AActor* interactor)
{
	return !itemId.IsNone() && quantity > 0;
}

void AWorldItem::InitializeItem(FName inItemId, int32 inQuantity, float inDurability)
{
	itemId = inItemId;
	quantity = FMath::Max(1, inQuantity);
	durability = inDurability;

	ApplyPresentation();
}

void AWorldItem::BeginPlay()
{
	Super::BeginPlay();
	// 회복 아이템은 플레이어 귀속이라 필드에 놓이지 않는다 (사용자 2026-11-17)
	if (itemId == GetDefault<UInventoryComponent>()->recoveryItemId)
	{
		Destroy();
		return;
	}
	ApplyPresentation();

	FItemData data;
}

bool AWorldItem::LookupItemData(FItemData& outData) const
{
	if (itemId.IsNone())
		return false;

	const UItemDatabaseSubsystem* database = UItemDatabaseSubsystem::Get(this);
	if (!database)
		return false;

	return database->GetItemData(itemId, outData);
}

void AWorldItem::ApplyPresentation()
{
	FItemData data;
	const bool bFound = LookupItemData(data);

	const EItemGrade grade = bFound ? data.grade : EItemGrade::EIG_Common;
	const FLinearColor gradeColor = FLinearColor::FromSRGBColor(GetItemGradeColor(grade));

	if (itemCollision)
		itemCollision->SetSphereRadius(collisionRadius * presentationScale, false);

	if (interactionSphere)
		interactionSphere->SetSphereRadius(interactionRadius, false);

	/* 빛줄기 이펙트 */
	if (beamEffect)
	{
		if (fieldEffectAsset && beamEffect->GetAsset() != fieldEffectAsset)
			beamEffect->SetAsset(fieldEffectAsset);

		beamEffect->SetVariableLinearColor(beamColorParamName, gradeColor);
		beamEffect->SetVariableFloat(beamScaleParamName, presentationScale);

		if (!beamEffect->IsActive())
			beamEffect->Activate(true);
	}

	/* 조명 */
	if (beamLight)
	{
		beamLight->SetLightColor(gradeColor);
		beamLight->SetIntensity(baseLightIntensity * presentationScale);
		beamLight->SetAttenuationRadius(baseLightRadius * presentationScale);
	}

	/* 필드 메시 */
	if (itemMesh)
	{
		UStaticMesh* mesh = (bFound && !data.fieldMesh.IsNull()) ? data.fieldMesh.LoadSynchronous() : nullptr;
		itemMesh->SetStaticMesh(mesh);
		itemMesh->SetRelativeScale3D(FVector(presentationScale));
		itemMesh->SetVisibility(bShowItemMesh && mesh != nullptr);
	}
}

void AWorldItem::PlayPickupEffect()
{
	if (pickupSound)
		UGameplayStatics::SpawnSoundAtLocation(this, pickupSound, GetActorLocation());

	if (pickupEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, pickupEffect, GetActorLocation());
}