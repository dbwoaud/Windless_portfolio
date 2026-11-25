#include "Components/WeakPointComponent.h"
#include "Components/WidgetComponent.h" 
#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WeakPointRoll.h"
#include "Characters/BaseCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "UI/WeakPoint/WeakPointMarkerWidget.h"
#include "Windless/DebugMacro.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"         

UWeakPointComponent::UWeakPointComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UWeakPointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner() && GetOwner()->ActorHasTag(FName("Dead")))
	{
		SetComponentTickEnabled(false);
		return;
	}

	DrawWeakPointDebug();
}

FWeakPointHitResult UWeakPointComponent::EvaluateHit(const FCombatHitInfo& hitInfo) const
{
	FWeakPointHitResult result;

	USkeletalMeshComponent* ownerMesh = GetOwnerMesh();
	const bool bValidHitMesh = hitInfo.hitMesh && hitInfo.hitMesh == ownerMesh;
	const FName hitBone = bValidHitMesh ? hitInfo.hitBoneName : NAME_None;

	const int32 regionIndex = FindRegionIndex(ownerMesh, hitBone, hitInfo.impactPoint);
	if (regionIndex == INDEX_NONE)
		return result;

	result.regionName = candidateRegions[regionIndex].regionName;
	if (!activeRegionIndices.Contains(regionIndex))
		return result;

	result.bCritical = true;
	result.awakeningBonus = GetAttackerAwakeningBonus(hitInfo.attacker);
	result.multiplier = WindlessCombat::CalcWeakPointMultiplier(result.awakeningBonus);
	return result;
}

void UWeakPointComponent::RollWeakPoints()
{
	TArray<FName> regionNames;
	for (const FWeakPointRegion& region : candidateRegions)
		regionNames.Add(region.regionName);

	const FWeakPointRule rule = bHasOverrideRule ? overrideRule : BuildComponentRule();
	FRandomStream random(FMath::Rand());
	TArray<FName> missingNames;
	activeRegionIndices = WindlessWeakPoint::PickRegions(regionNames, rule, random, &missingNames);

	for (const FName& name : missingNames)
		UE_LOG(LogWindless, Warning, TEXT("[약점] %s 부위 '%s'가 후보 목록에 없음"), *GetNameSafe(GetOwner()), *name.ToString());

	FString names;
	for (const FName& name : GetActiveRegionNames())
		names += names.IsEmpty() ? name.ToString() : FString::Printf(TEXT(", %s"), *name.ToString());

	UE_LOG(LogWindless, Log, TEXT("[약점] 지정 %s (%s, %s): %s"), *GetNameSafe(GetOwner()),
		rule.bFixed ? TEXT("고정") : TEXT("무작위"), bHasOverrideRule ? TEXT("원형") : TEXT("컴포넌트"),
		names.IsEmpty() ? TEXT("없음") : *names);
}

void UWeakPointComponent::ApplyRule(const FWeakPointRule& rule)
{
	overrideRule = rule;
	bHasOverrideRule = true;

	if (HasBegunPlay())
		UE_LOG(LogWindless, Warning, TEXT("[약점] %s 규칙이 BeginPlay 뒤에 들어옴"), *GetNameSafe(GetOwner()));
}

TArray<FName> UWeakPointComponent::GetActiveRegionNames() const
{
	TArray<FName> names;
	for (const int32 index : activeRegionIndices)
		names.Add(candidateRegions[index].regionName);

	return names;
}

TArray<FName> UWeakPointComponent::GetActiveWeakBones() const
{
	TArray<FName> bones;
	for (const int32 index : activeRegionIndices)
		bones.Append(candidateRegions[index].boneNames);

	return bones;
}

void UWeakPointComponent::BeginPlay()
{
	Super::BeginPlay();

	BuildBoneMap();
	RollWeakPoints();
	CreateMarkers();
	SetComponentTickEnabled(bDrawDebug);

	if (UWorld* world = GetWorld())
		world->GetTimerManager().SetTimerForNextTick(this, &UWeakPointComponent::BindPlayerChaos);
}

void UWeakPointComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAttributeComponent* attributes = playerAttributes.Get())
		attributes->onChaosStateChanged.RemoveDynamic(this, &UWeakPointComponent::HandlePlayerChaosChanged);

	if (UWorld* world = GetWorld())
		world->GetTimerManager().ClearTimer(markerUpdateTimer);

	Super::EndPlay(EndPlayReason);
}

FWeakPointRule UWeakPointComponent::BuildComponentRule() const
{
	FWeakPointRule rule;
	rule.bFixed = bFixedWeakPoints;
	rule.fixedNames = fixedRegionNames;
	rule.minCount = minWeakPointCount;
	rule.maxCount = maxWeakPointCount;
	return rule;
}

void UWeakPointComponent::CreateMarkers()
{
	AActor* owner = GetOwner();
	USkeletalMeshComponent* mesh = GetOwnerMesh();
	if (!owner || !mesh)
		return;

	/* 각성 1단계 경고 마커 (머리 위, 위치 비공개) */
	if (activeRegionIndices.Num() > 0)
	{
		warningMarker = CreateMarkerComponent(warningMarkerClass, owner->GetRootComponent(), NAME_None, warningMarkerOffset);
		if (warningMarker)
		{
			if (UWeakPointMarkerWidget* widget = Cast<UWeakPointMarkerWidget>(warningMarker->GetUserWidgetObject()))
				widget->SetupWarning(activeRegionIndices.Num());
		}
	}

	/* 각성 2단계 이상 상세 마커 (부위마다 첫 번째 유효 본에 부착) */
	for (const int32 index : activeRegionIndices)
	{
		const FWeakPointRegion& region = candidateRegions[index];
		const FName* attachBone = region.boneNames.FindByPredicate([mesh](const FName& bone)
			{
				return mesh->GetBoneIndex(bone) != INDEX_NONE;
			});

		if (!attachBone)
			continue;

		UWidgetComponent* marker = CreateMarkerComponent(detailMarkerClass, mesh, *attachBone, FVector::ZeroVector);
		if (!marker)
			continue;

		if (UWeakPointMarkerWidget* widget = Cast<UWeakPointMarkerWidget>(marker->GetUserWidgetObject()))
			widget->SetupDetail(region.displayName.IsEmpty() ? FText::FromName(region.regionName) : region.displayName);

		detailMarkers.Add(marker);
	}
}

UWidgetComponent* UWeakPointComponent::CreateMarkerComponent(TSubclassOf<UWeakPointMarkerWidget> widgetClass, USceneComponent* parent, FName socketName, const FVector& offset)
{
	if (!widgetClass || !parent || !GetOwner())
		return nullptr;

	UWidgetComponent* marker = NewObject<UWidgetComponent>(GetOwner());
	marker->SetupAttachment(parent, socketName);
	marker->SetRelativeLocation(offset);
	marker->SetWidgetSpace(EWidgetSpace::Screen);
	marker->SetDrawAtDesiredSize(true);
	marker->SetWidgetClass(widgetClass);
	marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	marker->SetVisibility(false);
	marker->RegisterComponent();
	marker->InitWidget();
	return marker;
}

void UWeakPointComponent::UpdateMarkerVisibility()
{
	const AActor* owner = GetOwner();
	const bool bDead = !owner || owner->ActorHasTag(FName("Dead"));
	const APawn* player = UGameplayStatics::GetPlayerPawn(this, 0);
	const bool bInRange = player && owner &&
		FVector::DistSquared(player->GetActorLocation(), owner->GetActorLocation()) <= FMath::Square(markerVisibleDistance);
	const bool bShow = !bDead && bInRange;

	const APlayerCameraManager* camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	const FVector viewLocation = camera ? camera->GetCameraLocation() : FVector::ZeroVector;

	auto ShouldShowMarker = [&](const UWidgetComponent* marker, bool bStageMatch) -> bool
		{
			if (!marker || !bShow || !bStageMatch)
				return false;

			if (!bHideWhenOccluded || !camera)
				return true;

			return !IsMarkerOccluded(viewLocation, marker->GetComponentLocation());
		};

	if (warningMarker)
		warningMarker->SetVisibility(ShouldShowMarker(warningMarker, currentDisplay == EWeakPointDisplay::EWD_Warning));

	for (UWidgetComponent* marker : detailMarkers)
	{
		if (marker)
			marker->SetVisibility(ShouldShowMarker(marker, currentDisplay == EWeakPointDisplay::EWD_Detail));
	}

	if (bDead && GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(markerUpdateTimer);
}

bool UWeakPointComponent::IsMarkerOccluded(const FVector& viewLocation, const FVector& markerLocation) const
{
	UWorld* world = GetWorld();
	if (!world)
		return false;

	FCollisionQueryParams params(SCENE_QUERY_STAT(WeakPointMarkerSight), false, GetOwner());
	if (APawn* player = UGameplayStatics::GetPlayerPawn(this, 0))
		params.AddIgnoredActor(player);

	FHitResult hit;
	return world->LineTraceSingleByChannel(hit, viewLocation, markerLocation, occlusionTraceChannel, params);
}

EWeakPointDisplay UWeakPointComponent::GetDisplayForChaos(EChaosState state)
{
	switch (state)
	{
	case EChaosState::ECS_Awakening1: return EWeakPointDisplay::EWD_Warning;
	case EChaosState::ECS_Awakening2:
	case EChaosState::ECS_Awakening3: return EWeakPointDisplay::EWD_Detail;
	default:                          return EWeakPointDisplay::EWD_Hidden;
	}
}

void UWeakPointComponent::BindPlayerChaos()
{
	const ABaseCharacter* player = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	UAttributeComponent* attributes = player ? player->GetAttributes() : nullptr;
	if (!attributes)
		return;

	playerAttributes = attributes;
	attributes->onChaosStateChanged.AddUniqueDynamic(this, &UWeakPointComponent::HandlePlayerChaosChanged);
	HandlePlayerChaosChanged(attributes->GetChaosState());
}

void UWeakPointComponent::HandlePlayerChaosChanged(EChaosState newState)
{
	const EWeakPointDisplay newDisplay = GetDisplayForChaos(newState);
	if (newDisplay == currentDisplay)
		return;

	currentDisplay = newDisplay;

	if (UWorld* world = GetWorld())
	{
		FTimerManager& timerManager = world->GetTimerManager();
		if (currentDisplay == EWeakPointDisplay::EWD_Hidden)
			timerManager.ClearTimer(markerUpdateTimer);
		else if (!timerManager.IsTimerActive(markerUpdateTimer))
			timerManager.SetTimer(markerUpdateTimer, this, &UWeakPointComponent::UpdateMarkerVisibility, markerUpdateInterval, true);
	}

	UE_LOG(LogWindless, Log, TEXT("[약점] UI %s 표시단계=%d"), *GetNameSafe(GetOwner()), (int32)currentDisplay);
	UpdateMarkerVisibility();
}

void UWeakPointComponent::BuildBoneMap()
{
	boneToRegionMap.Reset();
	const USkeletalMeshComponent* mesh = GetOwnerMesh();

	for (int32 i = 0; i < candidateRegions.Num(); ++i)
	{
		for (const FName& bone : candidateRegions[i].boneNames)
		{
			if (mesh && mesh->GetBoneIndex(bone) == INDEX_NONE)
				UE_LOG(LogWindless, Warning, TEXT("[약점] %s 스켈레톤에 없는 본: %s (부위 %s)"),
					*GetNameSafe(GetOwner()), *bone.ToString(), *candidateRegions[i].regionName.ToString());

			boneToRegionMap.Add(bone, i);
		}
	}
}

int32 UWeakPointComponent::FindRegionIndex(const USkeletalMeshComponent* mesh, FName hitBone, const FVector& impactPoint) const
{
	if (!mesh)
		return INDEX_NONE;

	if (hitBone.IsNone())
		return FindClosestRegionIndex(mesh, impactPoint);

	/* 피격 본부터 부모 방향으로 올라가며 처음 만나는 등록 본으로 판정 */
	for (FName bone = hitBone; !bone.IsNone(); bone = mesh->GetParentBone(bone))
	{
		const int32* found = boneToRegionMap.Find(bone);
		if (!found)
			continue;

		const bool bExactBone = (bone == hitBone);
		return (bExactBone || candidateRegions[*found].bIncludeChildBones) ? *found : INDEX_NONE;
	}

	return INDEX_NONE;
}

int32 UWeakPointComponent::FindClosestRegionIndex(const USkeletalMeshComponent* mesh, const FVector& impactPoint) const
{
	int32 bestIndex = INDEX_NONE;
	float bestDistSq = FMath::Square(fallbackSearchRadius);

	for (const TPair<FName, int32>& pair : boneToRegionMap)
	{
		if (mesh->GetBoneIndex(pair.Key) == INDEX_NONE)
			continue;

		const float distSq = FVector::DistSquared(mesh->GetBoneLocation(pair.Key), impactPoint);
		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;
			bestIndex = pair.Value;
		}
	}

	return bestIndex;
}

float UWeakPointComponent::GetAttackerAwakeningBonus(const AActor* attacker) const
{
	const ABaseCharacter* character = Cast<ABaseCharacter>(attacker);
	const UAttributeComponent* attackerAttributes = character ? character->GetAttributes() : nullptr;
	return attackerAttributes ? attackerAttributes->GetWeaknessDamageMultiplier() : 1.f;
}

USkeletalMeshComponent* UWeakPointComponent::GetOwnerMesh() const
{
	const ACharacter* character = Cast<ACharacter>(GetOwner());
	return character ? character->GetMesh() : nullptr;
}

void UWeakPointComponent::DrawWeakPointDebug() const
{
	const USkeletalMeshComponent* mesh = GetOwnerMesh();
	UWorld* world = GetWorld();
	if (!mesh || !world)
		return;

	for (const FName& bone : GetActiveWeakBones())
	{
		if (mesh->GetBoneIndex(bone) != INDEX_NONE)
			DrawDebugSphere(world, mesh->GetBoneLocation(bone), debugSphereRadius, 8, FColor::Red, false, -1.f, 0, 1.f);
	}
}
