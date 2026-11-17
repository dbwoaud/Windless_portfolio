#include "Level/ChaosZoneVolume.h"
#include "Characters/WindlessCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"

AChaosZoneVolume::AChaosZoneVolume()
{
	bTriggerOnce = false;

	if (triggerBox)
	{
		triggerBox->SetBoxExtent(FVector(500.f, 500.f, 200.f));
		triggerBox->SetCanEverAffectNavigation(false);
	}
}

void AChaosZoneVolume::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);
	if (triggerBox) // 에디터에서 빛은 노랑, 어둠은 보라로 보인다
		triggerBox->ShapeColor = zone == EChaosZone::Light ? FColor::Yellow : FColor::Purple;
}

void AChaosZoneVolume::InitAsLight(float floor, float radius)
{
	zone = EChaosZone::Light;
	lightFloor = floor;
	if (triggerBox)
		triggerBox->SetBoxExtent(FVector(radius));

	UPointLightComponent* light = NewObject<UPointLightComponent>(this);
	light->SetupAttachment(RootComponent);
	light->SetAttenuationRadius(radius);
	light->RegisterComponent();
}

void AChaosZoneVolume::HandleTriggerEnter(AActor* activator)
{
	Super::HandleTriggerEnter(activator);
	if (AWindlessCharacter* player = Cast<AWindlessCharacter>(activator))
		player->AddChaosZone(this);
}

void AChaosZoneVolume::HandleTriggerExit(AActor* activator)
{
	Super::HandleTriggerExit(activator);
	if (AWindlessCharacter* player = Cast<AWindlessCharacter>(activator))
		player->RemoveChaosZone(this);
}
