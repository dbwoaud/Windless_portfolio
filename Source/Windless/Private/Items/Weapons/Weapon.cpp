#include "Items/Weapons/Weapon.h"
#include "Items/ItemTypes.h"
#include "Characters/WindlessCharacter.h"
#include "Characters/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interfaces/HitInterface.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "Windless/DebugMacro.h"

AWeapon::AWeapon()
{
	weaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	weaponBox->SetupAttachment(GetRootComponent());
	weaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	weaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	weaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	
	boxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	boxTraceStart->SetupAttachment(GetRootComponent());

	boxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	boxTraceEnd->SetupAttachment(GetRootComponent());

	heavyAttack.staminaCost = 20.f;
	heavyAttack.impact = 20.f;

	jumpAttack.staminaCost = 15.f;
	jumpAttack.impact = 25.f;
}

void AWeapon::Equip(USceneComponent* inParent, FName inSocketName, AActor* newOwner, APawn* newInstigator)
{
	itemState = EItemState::EIS_Equipped;
	SetOwner(newOwner); // 무기의 소유자를 설정하는 함수
	SetInstigator(newInstigator); // 무기 데미지와 이벤트의 책임자를 설정하는 함수
	AttachMeshToSocket(inParent, inSocketName);
	PlayEquipSound();
	DisableSphereCollision();
	DeactivateEmbers();
}

void AWeapon::ApplyItemData(const FItemData& data)
{
	sourceItemId = data.itemId;
	weaponType = data.weaponType;
	guardValue = data.guardValueBonus;
}

void AWeapon::AttachMeshToSocket(USceneComponent* inParent, const FName& inSocketName)
{
	FAttachmentTransformRules transformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(inParent, transformRules, inSocketName);
}

void AWeapon::PlayEquipSound()
{
	if (equipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			equipSound,
			GetActorLocation()
		);
	}
}

void AWeapon::DisableSphereCollision()
{
	if (sphere)
		sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::DeactivateEmbers()
{
	if (itemEffect)
		itemEffect->Deactivate();
}

void AWeapon::PlayGuardSound()
{
	if (guardSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			guardSound,
			GetActorLocation()
		);
	}
}

void AWeapon::SetHitOverride(const FWeaponHitOverride& inHitOverride)
{
	hitOverride = inHitOverride;
}

void AWeapon::ClearHitOverride()
{
	hitOverride.Reset();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	weaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);	
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner() || ActorIsSameType(OtherActor)) // overlap된 액터가 소유자이거나 아군인 경우 무시
		return;

	FHitResult boxHit;
	BoxTrace(boxHit);
	if (!boxHit.GetActor())
		return;

	if (ActorIsSameType(boxHit.GetActor())) // 박스 추적 대상이 아군이면 무시
		return;

	const bool bCanGetHit = ApplyHitToTarget(boxHit); // 타입별 피해 적용
	if (bCanGetHit)
		ExecuteGetHit(boxHit);

	CreateFields(boxHit.ImpactPoint);
}

bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
	return GetOwner()->ActorHasTag(FName("Enemy")) && OtherActor->ActorHasTag(FName("Enemy"));
}

float AWeapon::GetOwnerAttackPower() const
{
	const ABaseCharacter* ownerCharacter = Cast<ABaseCharacter>(GetOwner());
	return ownerCharacter ? ownerCharacter->GetAttackPowerWith(this) : 0.f;
}

void AWeapon::ExecuteGetHit(FHitResult& boxHit)
{
	IHitInterface* hitInterface = Cast<IHitInterface>(boxHit.GetActor());
	if (hitInterface)
		hitInterface->Execute_GetHit(boxHit.GetActor(), boxHit.ImpactPoint, GetOwner());
}


float AWeapon::CalculateCurrentDamage() const
{
	if (hitOverride.IsSet())
		return hitOverride->damage;

	return WindlessCombat::CalcHitDamage(GetOwnerAttackPower(), currentAttackType);
}

FCombatHitInfo AWeapon::BuildHitInfo(const FHitResult& boxHit)
{
	FCombatHitInfo hitInfo;
	hitInfo.attacker = GetOwner();
	hitInfo.causer = this;
	hitInfo.instigatorController = GetInstigator() ? GetInstigator()->GetController() : nullptr;
	hitInfo.hitMesh = Cast<USkeletalMeshComponent>(boxHit.GetComponent());
	hitInfo.hitBoneName = boxHit.BoneName;
	hitInfo.impactPoint = boxHit.ImpactPoint;
	hitInfo.attackType = currentAttackType;
	hitInfo.damage = CalculateCurrentDamage();
	hitInfo.impact = GetAttackSpec(currentAttackType).impact;
	hitInfo.injuryChance = injuryChance;
	hitInfo.bUnblockable = GetAttackSpec(currentAttackType).bUnblockable;

	if (hitOverride.IsSet()) // 몬스터 데이터 공격 사용
	{
		hitInfo.attackType = hitOverride->attackType;
		hitInfo.impact = hitOverride->impact;
		hitInfo.bUnblockable = false;
		hitInfo.attackId = hitOverride->attackId;
		hitInfo.windup = hitOverride->windup;
		hitInfo.bCanStaggerStrong = hitOverride->bCanStaggerStrong;
	}
	
	return hitInfo;
}

bool AWeapon::ApplyHitToTarget(const FHitResult& boxHit)
{
	ABaseCharacter* hitCharacter = Cast<ABaseCharacter>(boxHit.GetActor());
	if (hitCharacter)
		return ApplyHitToCharacter(hitCharacter, boxHit);

	AController* instigatorController = GetInstigator() ? GetInstigator()->GetController() : nullptr;
	UGameplayStatics::ApplyDamage(boxHit.GetActor(), CalculateCurrentDamage(), instigatorController, this, UDamageType::StaticClass());
	return true;
}

bool AWeapon::ApplyHitToCharacter(ABaseCharacter* hitCharacter, const FHitResult& boxHit)
{
	const FCombatHitInfo hitInfo = BuildHitInfo(boxHit);

	UE_LOG(LogWindless, Log, TEXT("[공격] %s -> %s 종류=%s 대미지=%.1f 충격량=%.1f%s"),
		*GetNameSafe(GetOwner()), *GetNameSafe(hitCharacter),
		WindlessCombat::GetAttackTypeName(hitInfo.attackType),
		hitInfo.damage, hitInfo.impact,
		hitInfo.attackId.IsNone() ? TEXT("") : *FString::Printf(TEXT(" (데이터 %s, 예고 %.2f초)"), *hitInfo.attackId.ToString(), hitInfo.windup));

	const bool bCanGetHit = hitCharacter->ReceiveWeaponHit(hitInfo);
	ABaseCharacter* attacker = Cast<ABaseCharacter>(GetOwner());
	if (attacker)
	{
		attacker->OnDealtWeaponHit();
		if (bCanGetHit)
			attacker->OnWeaponHitLanded();
	}

	return bCanGetHit;
}

void AWeapon::BoxTrace(FHitResult& boxHit)
{
	const FVector start = boxTraceStart->GetComponentLocation();
	const FVector end = boxTraceEnd->GetComponentLocation();

	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(this); // 박스 추적을 무시할 액터에 무기 자신을 추가
	actorsToIgnore.Add(GetOwner()); // 박스 추적을 무시할 액터에 무기 소유자를 추가

	for (AActor* actor : ignoreActors)
		actorsToIgnore.AddUnique(actor); // 박스 추적을 무시할 액터에 사용자가 설정한 액터를 추가

	UKismetSystemLibrary::BoxTraceSingle(
		this,
		start,
		end,
		boxTraceExtent,
		boxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		actorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		boxHit,
		true
	);	

	ignoreActors.AddUnique(boxHit.GetActor()); // 중복 타격을 방지하기 위해 타격 당한 액터를 추가
}