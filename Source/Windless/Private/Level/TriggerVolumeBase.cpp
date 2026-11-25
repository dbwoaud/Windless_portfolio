#include "Level/TriggerVolumeBase.h"
#include "Windless/DebugMacro.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ATriggerVolumeBase::ATriggerVolumeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	triggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	triggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	triggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	triggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	triggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	triggerBox->SetGenerateOverlapEvents(true);
	triggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	RootComponent = triggerBox;
}

void ATriggerVolumeBase::BeginPlay()
{
	Super::BeginPlay();
	triggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATriggerVolumeBase::OnBoxBeginOverlap);
	triggerBox->OnComponentEndOverlap.AddDynamic(this, &ATriggerVolumeBase::OnBoxEndOverlap);
}

void ATriggerVolumeBase::ResetTrigger()
{
	bHasTriggered = false;
	UE_LOG(LogWindless, Log, TEXT("트리거 리셋 : %s"), *GetName());
}

void ATriggerVolumeBase::HandleTriggerEnter(AActor* activator)
{

}

void ATriggerVolumeBase::HandleTriggerExit(AActor* activator)
{

}

void ATriggerVolumeBase::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidActivator(OtherActor))
		return;

	if (bTriggerOnce && bHasTriggered) // 1회성 트리거인 경우 무시 
		return;

	bHasTriggered = true;

	UE_LOG(LogWindless, Log, TEXT("트리거 진입: %s -> %s"), *OtherActor->GetName(), *GetName());

	HandleTriggerEnter(OtherActor); 
	OnTriggerEnterBP(OtherActor);
	onTriggerEnter.Broadcast(OtherActor);
}

void ATriggerVolumeBase::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValidActivator(OtherActor))
		return;

	UE_LOG(LogWindless, Log, TEXT("트리거 이탈: %s -> %s"), *OtherActor->GetName(), *GetName());

	HandleTriggerExit(OtherActor);
	OnTriggerExitBP(OtherActor);
	onTriggerExit.Broadcast(OtherActor);
}

bool ATriggerVolumeBase::IsValidActivator(AActor* other) const
{
	if (!other)
		return false;

	if (bPlayerOnly)
	{
		ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
		if (other != playerCharacter)
			return false;
	}

	if (requiredActorTag != NAME_None && !other->ActorHasTag(requiredActorTag))
		return false;

	return true;
}