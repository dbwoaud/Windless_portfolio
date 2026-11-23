#include "Characters/WindlessCharacter.h"
#include "Level/ChaosZoneVolume.h"
#include "Characters/BodyPartTypes.h"
#include "Characters/LevelUpTypes.h"
#include "Characters/CombatTypes.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TargetingComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/CharacterDebugComponent.h"
#include "Components/MountComponent.h"
#include "Components/CombatComponent.h"
#include "Components/DialogueComponent.h"
#include "Components/QuickSlotComponent.h"
#include "Components/MaintenanceComponent.h"
#include "Components/RefineComponent.h"
#include "Components/RepairComponent.h"
#include "Components/CraftingComponent.h"
#include "Components/FastTravelComponent.h"
#include "Components/WeaponLoadoutComponent.h"
#include "Components/BowComponent.h"
#include "Components/PlayerAISignal.h" 
#include "Components/PlayerStaggerComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h" 
#include "GroomComponent.h"
#include "Items.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/DropItem.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "HUD/WindlessHUD.h"
#include "HUD/WindlessOverlay.h"
#include "UI/InteractionWidget.h"
#include "UI/InventoryWidget.h"
#include "Interfaces/PickupInterface.h"
#include "Interfaces/InteractableInterface.h" 
#include "Blueprint/UserWidget.h"
#include "Windless/DebugMacro.h"
#include "Camera/CameraShakeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Save/WindlessSaveSubsystem.h"
#include "Level/StorageContainer.h"

AWindlessCharacter::AWindlessCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 150.f; // 기준치수표 7.1 D2: 웅크려 걷기
	GetCharacterMovement()->JumpZVelocity = 343.f; // 기준치수표 7.1 D4: 정점 0.6 m, 체공 0.70초 (중력 1배)

	safeFallDistance = 300.f; // 기준치수표 D7: 플레이어는 3 m까지 낙하 피해 없음 (적은 BaseCharacter 기본값 5 m)

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	springArm->SetupAttachment(GetRootComponent());
	springArm->TargetArmLength = 300.f;

	viewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	viewCamera->SetupAttachment(springArm);

	hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	hair->SetupAttachment(GetMesh());
	hair->AttachmentName = FString("head");

	eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	eyebrows->SetupAttachment(GetMesh());
	eyebrows->AttachmentName = FString("head");

	targetingComponent = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComponent"));

	inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	debugComponent = CreateDefaultSubobject<UCharacterDebugComponent>(TEXT("CharacterDebug"));

	mountComponent = CreateDefaultSubobject<UMountComponent>(TEXT("MountComponent"));

	combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));

	dialogueComponent = CreateDefaultSubobject<UDialogueComponent>(TEXT("DialogueComponent"));

	quickSlots = CreateDefaultSubobject<UQuickSlotComponent>(TEXT("QuickSlotComponent"));

	maintenance = CreateDefaultSubobject<UMaintenanceComponent>(TEXT("Maintenance"));

	refine = CreateDefaultSubobject<URefineComponent>(TEXT("Refine"));

	repair = CreateDefaultSubobject<URepairComponent>(TEXT("Repair"));

	crafting = CreateDefaultSubobject<UCraftingComponent>(TEXT("Crafting"));

	fastTravel = CreateDefaultSubobject<UFastTravelComponent>(TEXT("FastTravel"));

	weaponLoadout = CreateDefaultSubobject<UWeaponLoadoutComponent>(TEXT("WeaponLoadout"));

	bow = CreateDefaultSubobject<UBowComponent>(TEXT("Bow"));

	staggerComponent = CreateDefaultSubobject<UPlayerStaggerComponent>(TEXT("Stagger"));

	aiSignals = CreateDefaultSubobject<UPlayerAISignal>(TEXT("AISignals"));
}

void AWindlessCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleStamina(DeltaTime);
	HandleBodyRegen(DeltaTime);
	HandleChaos(DeltaTime);
	HandleLegFractureInjury(DeltaTime);
	RefreshQuickSlotCooldownUI();
	HandleAirRotationLock();
	RefreshRotationMode();
	UpdateAttackFacing(DeltaTime);
}

void AWindlessCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(movementAction, ETriggerEvent::Triggered, this, &AWindlessCharacter::Move);
		EnhancedInputComponent->BindAction(movementAction, ETriggerEvent::Completed, this, &AWindlessCharacter::StopMoving);
		EnhancedInputComponent->BindAction(lookAction, ETriggerEvent::Triggered, this, &AWindlessCharacter::Look);
		EnhancedInputComponent->BindAction(sprintAction, ETriggerEvent::Started, this, &AWindlessCharacter::Sprint);
		EnhancedInputComponent->BindAction(sprintAction, ETriggerEvent::Completed, this, &AWindlessCharacter::StopSprinting);
		EnhancedInputComponent->BindAction(jumpAction, ETriggerEvent::Triggered, this, &AWindlessCharacter::Jump);
		EnhancedInputComponent->BindAction(dodgeAction, ETriggerEvent::Started, this, &AWindlessCharacter::OnSpacePressed);
		EnhancedInputComponent->BindAction(equipAction, ETriggerEvent::Triggered, this, &AWindlessCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(leftAttackAction, ETriggerEvent::Started, this, &AWindlessCharacter::Attack);
		EnhancedInputComponent->BindAction(leftAttackAction, ETriggerEvent::Triggered, this, &AWindlessCharacter::LeftAttackHeld);
		EnhancedInputComponent->BindAction(leftAttackAction, ETriggerEvent::Completed, this, &AWindlessCharacter::LeftAttackReleased);
		EnhancedInputComponent->BindAction(rightAttackAction, ETriggerEvent::Started, this, &AWindlessCharacter::RightAttack);
		EnhancedInputComponent->BindAction(rightAttackAction, ETriggerEvent::Triggered, this, &AWindlessCharacter::RightAttackHeld);
		EnhancedInputComponent->BindAction(rightAttackAction, ETriggerEvent::Completed, this, &AWindlessCharacter::RightAttackReleased);
		EnhancedInputComponent->BindAction(leftStrongAttackAction, ETriggerEvent::Started, this, &AWindlessCharacter::LeftStrongAttack);
		EnhancedInputComponent->BindAction(rightStrongAttackAction, ETriggerEvent::Started, this, &AWindlessCharacter::RightStrongAttack);
		EnhancedInputComponent->BindAction(crouchAction, ETriggerEvent::Started, this, &AWindlessCharacter::ToggleCrouch);
		EnhancedInputComponent->BindAction(lockOnAction, ETriggerEvent::Started, this, &AWindlessCharacter::OnLockOnPressed);
		EnhancedInputComponent->BindAction(switchTargetAction, ETriggerEvent::Triggered, this, &AWindlessCharacter::OnSwitchTargetTriggered);
		EnhancedInputComponent->BindAction(mountAction, ETriggerEvent::Started, this, &AWindlessCharacter::ToggleMount);
		EnhancedInputComponent->BindAction(bodyStatusToggleAction, ETriggerEvent::Started, this, &AWindlessCharacter::ToggleBodyStatus);
		EnhancedInputComponent->BindAction(inventoryToggleAction, ETriggerEvent::Started, this, &AWindlessCharacter::ToggleInventory);
		EnhancedInputComponent->BindAction(interactAction, ETriggerEvent::Started, this, &AWindlessCharacter::Interact);
		EnhancedInputComponent->BindAction(quickSlotRecoveryAction, ETriggerEvent::Started, this, &AWindlessCharacter::PressQuickSlotRecovery);
		EnhancedInputComponent->BindAction(quickSlotRecoveryAction, ETriggerEvent::Completed, this, &AWindlessCharacter::ReleaseQuickSlotRecovery);
		EnhancedInputComponent->BindAction(quickSlotConsume1Action, ETriggerEvent::Started, this, &AWindlessCharacter::PressQuickSlotConsume1);
		EnhancedInputComponent->BindAction(quickSlotConsume1Action, ETriggerEvent::Completed, this, &AWindlessCharacter::ReleaseQuickSlotConsume1);
		EnhancedInputComponent->BindAction(quickSlotConsume2Action, ETriggerEvent::Started, this, &AWindlessCharacter::PressQuickSlotConsume2);
		EnhancedInputComponent->BindAction(quickSlotConsume2Action, ETriggerEvent::Completed, this, &AWindlessCharacter::ReleaseQuickSlotConsume2);
		EnhancedInputComponent->BindAction(quickSlotConsume3Action, ETriggerEvent::Started, this, &AWindlessCharacter::PressQuickSlotConsume3);
		EnhancedInputComponent->BindAction(quickSlotConsume3Action, ETriggerEvent::Completed, this, &AWindlessCharacter::ReleaseQuickSlotConsume3);
		EnhancedInputComponent->BindAction(quickSlotConsume4Action, ETriggerEvent::Started, this, &AWindlessCharacter::PressQuickSlotConsume4);
		EnhancedInputComponent->BindAction(quickSlotConsume4Action, ETriggerEvent::Completed, this, &AWindlessCharacter::ReleaseQuickSlotConsume4);
		EnhancedInputComponent->BindAction(quickSlotCancelAction, ETriggerEvent::Started, this, &AWindlessCharacter::QuickSlotCancel);

		if (debugComponent)
			debugComponent->BindDebugInputs(EnhancedInputComponent);

		if (dialogueComponent)
			dialogueComponent->BindDialogueInputs(EnhancedInputComponent);
	}
}

void AWindlessCharacter::SetDialogueInputMode(bool bEnable)
{
	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (!playerController)
		return;

	UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer());
	if (!subsystem)
		return;

	if (bEnable) // 대화 모드 
	{
		SetInteractionInputMode(false);
		if (interactionWidget)
			interactionWidget->SetVisibility(ESlateVisibility::Collapsed);

		subsystem->RemoveMappingContext(WindlessContext);
		if (dialogueContext)
			subsystem->AddMappingContext(dialogueContext, 1);

		actionState = EActionState::EAS_Unoccupied;
		UE_LOG(LogWindless, Log, TEXT("대화 모드 ON"));
	}
	else // 입력 모드
	{
		if (dialogueContext)
			subsystem->RemoveMappingContext(dialogueContext);
		subsystem->AddMappingContext(WindlessContext, 0);

		UpdateInteractionTarget();
		UE_LOG(LogWindless, Log, TEXT("[입력] 대화 모드 OFF"));
	}
}

void AWindlessCharacter::SetInteractionInputMode(bool bEnable)
{
	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (!playerController)
		return;

	UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer());
	if (!subsystem || !interactionContext)
		return;

	if (bEnable && !bInteractionContextActive)
	{
		subsystem->AddMappingContext(interactionContext, 1);
		bInteractionContextActive = true;
	}
	else if (!bEnable && bInteractionContextActive)
	{
		subsystem->RemoveMappingContext(interactionContext);
		bInteractionContextActive = false;
	}
}

void AWindlessCharacter::SetInventoryInputMode(bool bEnable)
{
	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (!playerController)
		return;

	UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer());
	if (!subsystem)
		return;

	if (bEnable) // 인벤토리 조작 모드
	{
		SetInteractionInputMode(false);
		if (interactionWidget)
			interactionWidget->SetVisibility(ESlateVisibility::Collapsed);

		StopMoving();
		StopSprinting();
		StopGuard();
		if (bow)
			bow->CancelDraw(TEXT("인벤토리"));

		subsystem->RemoveMappingContext(WindlessContext);
		if (inventoryContext)
			subsystem->AddMappingContext(inventoryContext, 1);
	}
	else
	{
		if (inventoryContext)
			subsystem->RemoveMappingContext(inventoryContext);

		subsystem->AddMappingContext(WindlessContext, 0);
	}
}

bool AWindlessCharacter::IsOccupied()
{
	return actionState != EActionState::EAS_Unoccupied;
}

bool AWindlessCharacter::CanStartMovementAction(EMovementAction action, bool bIgnoreReleasableState)
{
	if (bIsDead || bIsMounted) // 사망 또는 탑승 중이면 모든 이동 액션 불가
		return false;

	const bool bReleasable = bIgnoreReleasableState &&
		(actionState == EActionState::EAS_Blocking || actionState == EActionState::EAS_Aiming);

	if (IsOccupied() && !bReleasable)
		return false;

	if (IsInMovement()) // 다른 이동 액션이 진행 중이면 불가
		return false;

	switch (action)
	{
	case EMovementAction::EMA_Jump:
		return !IsCrouchRequested();

	case EMovementAction::EMA_Dodge:
		return !IsCrouchRequested() || HasStandingHeadroom();

	case EMovementAction::EMA_Crouch:
		return !IsCrouchRequested();

	default:
		return false;
	}
}

bool AWindlessCharacter::IsInMovement() const
{
	const UCharacterMovementComponent* movement = GetCharacterMovement();
	if (!movement)
		return false;

	if (movement->IsFalling()) // 점프 또는 낙하로 공중에 있는 경우
		return true;

	if (actionState == EActionState::EAS_Dodge) // 구르기 중인 경우
		return true;

	return false;
}

bool AWindlessCharacter::IsCrouchRequested() const
{
	const UCharacterMovementComponent* movement = GetCharacterMovement();
	return bIsCrouched || (movement && movement->bWantsToCrouch);
}

bool AWindlessCharacter::HasStandingHeadroom() const
{
	const UCapsuleComponent* capsule = GetCapsuleComponent();
	const UWorld* world = GetWorld();
	if (!capsule || !world)
		return true;

	if (!bIsCrouched) // 이미 서 있으면 판정 불필요
		return true;

	const ACharacter* defaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
	if (!defaultCharacter || !defaultCharacter->GetCapsuleComponent())
		return true;

	const float scale = capsule->GetShapeScale();
	const float radius = capsule->GetUnscaledCapsuleRadius() * scale;
	const float crouchedHalfHeight = capsule->GetUnscaledCapsuleHalfHeight() * scale;
	const float standingHalfHeight = defaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * scale;

	if (standingHalfHeight <= crouchedHalfHeight)
		return true;

	const FVector standCenter = GetActorLocation() + FVector(0.f, 0.f, standingHalfHeight - crouchedHalfHeight);
	const FCollisionShape standShape = FCollisionShape::MakeCapsule(radius, standingHalfHeight);

	FCollisionQueryParams params(SCENE_QUERY_STAT(WindlessStandingHeadroom), false, this);
	params.AddIgnoredActor(this);

	const bool bBlocked = world->OverlapBlockingTestByChannel(
		standCenter,
		FQuat::Identity,
		capsule->GetCollisionObjectType(),
		standShape,
		params
	);

	return !bBlocked;
}

bool AWindlessCharacter::IsBowEquipped() const
{
	return (rightHandWeapon && rightHandWeapon->IsBow()) || (leftHandWeapon && leftHandWeapon->IsBow());
}

void AWindlessCharacter::Jump()
{
	if (!CanStartMovementAction(EMovementAction::EMA_Jump))
		return;

	RotateToMovementInputDirection();
	LockAirRotation();

	Super::Jump();
}

void AWindlessCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	UnlockAirRotation();

	if (combat)
		combat->OnOwnerLanded();
}

float AWindlessCharacter::GetAttackPowerWith(const AWeapon* weapon) const
{
	if (!attributes)
		return 0.f;

	float weaponBonus = 0.f;
	if (inventory && weapon)
	{
		if (weapon == leftHandWeapon)
			weaponBonus = inventory->GetHandAttackBonus(EEquipSlot::EES_LeftHand);
		else if (weapon == rightHandWeapon)
			weaponBonus = inventory->GetHandAttackBonus(EEquipSlot::EES_RightHand);
	}

	return attributes->GetAttackPowerWithWeaponBonus(weaponBonus);
}

void AWindlessCharacter::OnDealtWeaponHit()
{
	if (aiSignals) 
		aiSignals->NotifyAttackHit();

	MarkCombatEngaged();

	if (!bodyParts)
		return;

	EBodyPart attackingArm = EBodyPart::EBP_None;
	if (currentAttackWeapon == GetLeftHandWeapon())
		attackingArm = EBodyPart::EBP_LeftArm;
	else if (currentAttackWeapon == GetRightHandWeapon())
		attackingArm = EBodyPart::EBP_RightArm;

	bodyParts->OnOwnerDealtHit(attackingArm);
}

void AWindlessCharacter::OnDealtRangedHit()
{
	MarkCombatEngaged();
}

bool AWindlessCharacter::CanAttack()
{
	return actionState == EActionState::EAS_Unoccupied &&
		characterState != ECharacterState::ECS_Unequipped &&
		!bIsMounted &&
		!bQuickSlotActive;
}

void AWindlessCharacter::HandleDamage(float damageAmount)
{
	
}

void AWindlessCharacter::BeginAttack()
{
	ABaseCharacter::Attack();
}

bool AWindlessCharacter::PerformAttack(AWeapon* weapon, EAttackType attackType, UAnimMontage* montage)
{
	if (!weapon || !montage)
		return false;

	currentAttackWeapon = weapon;
	currentAttackWeapon->SetCurrentAttackType(attackType);
	ConsumeStamina(weapon->GetAttackSpec(attackType).staminaCost);
	FaceAttackDirection();
	PlayAttackMontageSingle(montage, attackType == EAttackType::EAT_Heavy);

	if (aiSignals) 
		aiSignals->NotifyAttackBegin();

	actionState = EActionState::EAS_Attacking;
	return true;
}

bool AWindlessCharacter::JumpToAttackSection(UAnimMontage* montage, FName sectionName)
{
	UAnimInstance* anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!montage || !anim || !anim->Montage_IsActive(montage))
		return false;

	if (actionState != EActionState::EAS_Attacking)
		return false;

	if (anim->Montage_GetCurrentSection(montage) == sectionName)
		return false;

	anim->Montage_JumpToSection(sectionName, montage);
	return true;
}

void AWindlessCharacter::EnterGuardState()
{
	StopSprinting();
	actionState = EActionState::EAS_Blocking;
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = guardWalkSpeed;

	if (aiSignals)
		aiSignals->NotifyGuard(true);
}

void AWindlessCharacter::ExitGuardState()
{
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
	if (actionState == EActionState::EAS_Blocking)
		actionState = EActionState::EAS_Unoccupied;

	if (aiSignals)
		aiSignals->NotifyGuard(false);
}

void AWindlessCharacter::MarkCombatEngaged()
{
	if (!IsInCombatState())
		UE_LOG(LogWindless, Log, TEXT("[전투] 전투 상태 진입 (%.0f초 유지)"), combatStateDuration);

	lastDamagedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

bool AWindlessCharacter::IsInCombatState() const
{
	const UWorld* world = GetWorld();
	if (!world)
		return false;

	return (world->GetTimeSeconds() - lastDamagedTime) < combatStateDuration;
}

float AWindlessCharacter::GetTimeSinceLastDamage() const
{
	const UWorld* world = GetWorld();
	if (!world)
		return 0.f;

	return FMath::Max(0.f, combatStateDuration - (world->GetTimeSeconds() - lastDamagedTime));
}

bool AWindlessCharacter::ContinueAttackCombo(AWeapon* weapon, EAttackType attackType, UAnimMontage* montage, FName sectionName)
{
	UAnimInstance* anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!weapon || !montage || !anim)
		return false;
	
	if (!anim->Montage_IsActive(montage))
		return false;

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	currentAttackWeapon = weapon;
	currentAttackWeapon->SetCurrentAttackType(attackType);
	ConsumeStamina(weapon->GetAttackSpec(attackType).staminaCost);
	FaceAttackDirection();
	anim->Montage_JumpToSection(sectionName, montage);

	if (aiSignals) 
		aiSignals->NotifyAttackBegin();

	return true;
}

void AWindlessCharacter::FaceAttackDirection()
{
	const AActor* lockTarget = (targetingComponent && targetingComponent->IsLockedOn()) ? targetingComponent->GetCurrentTarget() : nullptr;

	if (lockTarget) // 락온 
	{
		const FVector toTarget = (lockTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (toTarget.IsNearlyZero())
			return;

		attackFacingYaw = toTarget.Rotation().Yaw;
	}
	else if (Controller) // 카메라 정면
		attackFacingYaw = Controller->GetControlRotation().Yaw;
	else
		return;

	UE_LOG(LogWindless, Log, TEXT("[공격] 방향 정렬 (%s, Yaw %.0f)"), lockTarget ? TEXT("락온 대상") : TEXT("시야"), attackFacingYaw);

	if (attackTurnDuration <= 0.f)
	{
		SetActorRotation(FRotator(0.f, attackFacingYaw, 0.f));
		attackTurnRemaining = 0.f;
		return;
	}

	attackTurnRemaining = attackTurnDuration;
}

bool AWindlessCharacter::ReceiveWeaponHit(const FCombatHitInfo& hitInfo)
{
	AActor* attacker = hitInfo.attacker ? hitInfo.attacker : hitInfo.causer;

	if (staggerComponent && staggerComponent->IsStrongStaggered())
	{
		UE_LOG(LogWindless, Log, TEXT("[경직] 강한 경직 중 피격 무시 (%s)"), *GetNameSafe(attacker));
		return false;
	}

	if (IsInvincible())
	{
		if (actionState == EActionState::EAS_Dodge && !bPerfectDodgeConsumed)
		{
			bPerfectDodgeConsumed = true;
			NotifyPerfectDodge();
			UE_LOG(LogWindless, Log, TEXT("[회피] 퍼펙트 회피 성립 - 혼돈 %.1f"), attributes ? attributes->GetChaos() : 0.f);
		}
		else
			UE_LOG(LogWindless, Log, TEXT("[회피] 무적 프레임 - 피격 무효"));

		return false;
	}

	SetPendingHit(hitInfo);
	float residual = FMath::Max(0.f, hitInfo.damage);

	FGuardResult guardResult;
	if (combat && combat->HandleGuard(attacker, hitInfo, guardResult))
	{
		residual = guardResult.residualDamage;
		if (guardResult.bGuardBroken) // 가드 브레이크
			ApplyGuardBreakStagger();
		else
			return false; // 가드 성공
	}

	if (bodyParts && residual > 0.f)
		bodyParts->CalculateIncomingDamage(hitInfo.hitMesh, hitInfo.impactPoint, residual, hitInfo.injuryChance);

	return true; // 가드 실패
}

void AWindlessCharacter::GetHit_Implementation(const FVector& impactPoint, AActor* hitter)
{
	MarkCombatEngaged();

	if (combat && combat->IsGuardingAgainst(hitter)) // 방어 각도 안에서 막은 공격은 피격 처리 생략
		return;

	if (combat && combat->IsGuarding()) // 방어 중이지만 각도를 벗어난 공격이면 방어 해제
		StopGuard();

	if (IsInventoryOpen()) // 맞으면 가방이 닫힘(D-40)
		if (APlayerController* controller = Cast<APlayerController>(GetController()))
			CloseInventory(controller);

	Super::GetHit_Implementation(impactPoint, hitter); // 경직/버팀 판정
}

float AWindlessCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	HandleDamage(ActualDamage);

	if (!IsAlive())
		Die();

	return ActualDamage;
}

bool AWindlessCharacter::IsAlive()
{
	if (bodyParts)
		return !bodyParts->IsDeadByParts();

	return Super::IsAlive();
}

void AWindlessCharacter::DieByFall()
{
	UE_LOG(LogWindless, Log, TEXT("[낙사] 플레이어 낙사"));
	ForceDeath();
}

void AWindlessCharacter::Revive()
{
	bIsDead = false;
	Tags.Remove(FName("Dead"));
	ClearDodgeState();
	if (staggerComponent)
		staggerComponent->CancelStagger();

	actionState = EActionState::EAS_Unoccupied;
	lastDamagedTime = -1000.f;

	if (bodyParts)
		bodyParts->HealAllParts();
	if (attributes)
		attributes->ResetChaos();

	if (UCapsuleComponent* capsule = GetCapsuleComponent())
		capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	if (USkeletalMeshComponent* mesh = GetMesh())
		mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	RefillRecoveryItem();

	UE_LOG(LogWindless, Log, TEXT("플레이어 부활"));
}

void AWindlessCharacter::ForceDeath()
{
	if (IsAlive() && !bIsDead)
		Die();
}

void AWindlessCharacter::SetOverlappingItem(AItems* item)
{
	overlappingItem = item;
}

void AWindlessCharacter::SetHandWeapon(EEquipSlot slot, AWeapon* weapon)
{
	AWeapon*& handWeapon = (slot == EEquipSlot::EES_LeftHand) ? leftHandWeapon : rightHandWeapon;
	AWeapon* previousWeapon = handWeapon;
	if (previousWeapon == weapon)
		return;

	if (previousWeapon && previousWeapon == currentAttackWeapon)
		CancelCurrentAttack();

	if (combat)
		combat->OnHandWeaponChanged(previousWeapon);

	handWeapon = weapon;
	RefreshCharacterState();

	if (weapon && weapon->IsBow() && targetingComponent && targetingComponent->IsLockedOn())
		targetingComponent->ReleaseLockOn(TEXT("활 장착"));
}

void AWindlessCharacter::EnterAimState()
{
	StopSprinting();
	actionState = EActionState::EAS_Aiming;
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = aimWalkSpeed;

	if (aiSignals) 
		aiSignals->NotifyAim(true);
}

void AWindlessCharacter::ExitAimState()
{
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
	if (actionState == EActionState::EAS_Aiming)
		actionState = EActionState::EAS_Unoccupied;

	if (aiSignals)
		aiSignals->NotifyAim(false);
}

void AWindlessCharacter::BeginBowShotRecovery()
{
	actionState = EActionState::EAS_Attacking;
}

float AWindlessCharacter::GetLockOnRangeMultiplier() const
{
	return bodyParts ? bodyParts->GetLockOnRangeMultiplier() : 1.0f;
}

bool AWindlessCharacter::IsStrafing() const
{
	if (bIsMounted)
		return false;

	if (actionState == EActionState::EAS_Aiming)
		return true;

	if (!targetingComponent || !targetingComponent->IsLockedOn())
		return false;

	const bool bSprintMoving = bIsSprinting && !currentMovementInput.IsNearlyZero();
	return !bSprintMoving;
}

UTargetingComponent* AWindlessCharacter::GetTargeting() const
{
	return targetingComponent;
}

float AWindlessCharacter::GetCurrentStamina() const
{
	return attributes ? attributes->GetStamina() : 0.f;
}

void AWindlessCharacter::ConsumeStamina(float cost)
{
	if (!attributes)
		return;

	attributes->UseStamina(cost);
	SetHUDStamina();
}

bool AWindlessCharacter::HasStamina(float cost) const
{
	return attributes && attributes->GetStamina() >= cost;
}

bool AWindlessCharacter::HasEnoughAttackStamina(const AWeapon* weapon, EAttackType attackType) const
{
	return weapon && HasStamina(weapon->GetAttackSpec(attackType).staminaCost);
}

void AWindlessCharacter::AddGold(ATreasure* treasure)
{
	if (attributes && treasure)
	{
		attributes->AddMoltStones(treasure->GetGold());
	}
}

void AWindlessCharacter::AddSouls(ASoul* soul)
{
	if (attributes && soul)
	{
		attributes->AddMoltStones(soul->GetSouls());
	}
}

bool AWindlessCharacter::CheckActionDelay() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() < actionUnlockTime;
}

void AWindlessCharacter::ApplyActionDelay()
{
	if (!bodyParts || !GetWorld())
		return;

	const float delay = bodyParts->GetActionDelayPenalty();
	if (delay > 0.f)
		actionUnlockTime = GetWorld()->GetTimeSeconds() + delay;
}

bool AWindlessCharacter::IsInventoryOpen() const
{
	return inventoryWidget && inventoryWidget->IsInViewport();
}

bool AWindlessCharacter::BeginItemCast(int64 instanceUID, EBodyPart targetPart, float castTime)
{
	if (actionState != EActionState::EAS_Unoccupied)
		return false;

	/* 시전 시간이 없는 아이템은 즉시 사용 처리 */
	if (castTime <= 0.f)
	{
		if (!inventory)
			return false;

		const FItemInstance inst = inventory->FindInstance(instanceUID);
		const FName usedItemId = inst.itemId;

		const bool bUsed = (targetPart == EBodyPart::EBP_None)
			? inventory->UseItem(instanceUID)
			: inventory->UseItemOnPart(instanceUID, targetPart);

		if (bUsed && quickSlots)
			quickSlots->NotifyItemUsed(usedItemId);

		return bUsed;
	}

	castingItemUID = instanceUID;
	castingTargetPart = targetPart;
	actionState = EActionState::EAS_UsingItem;

	if (aiSignals) 
		aiSignals->NotifyItemUse(true);

	float speedRate = 1.f;
	if (inventory)
		speedRate = inventory->GetItemMoveSpeedRate(instanceUID);
	ApplyCastMovementRestriction(speedRate);

	GetWorldTimerManager().SetTimer(itemCastTimer, this, &AWindlessCharacter::FinishItemCast, castTime, false);

	UE_LOG(LogWindless, Log, TEXT("[시전] 시작 (%.2f초)"), castTime);
	return true;
}

void AWindlessCharacter::CancelItemCast(bool bConsumeItem)
{
	if (actionState != EActionState::EAS_UsingItem)
		return;

	GetWorldTimerManager().ClearTimer(itemCastTimer);
	GetWorldTimerManager().ClearTimer(itemCastRecoveryTimer);

	if (bConsumeItem && inventory && castingItemUID != 0)
		inventory->RemoveInstance(castingItemUID, 1);

	castingItemUID = 0;
	castingTargetPart = EBodyPart::EBP_None;

	ClearCastMovementRestriction();

	if (aiSignals) 
		aiSignals->NotifyItemUse(false);

	actionState = EActionState::EAS_Unoccupied;
}

void AWindlessCharacter::SpawnDroppedItem()
{
	GetWorldTimerManager().ClearTimer(dropFallbackTimer);
	if (!pendingDropInstance.IsValid())
		return;

	UWorld* world = GetWorld();
	if (!world || !dropItemClass)
	{
		pendingDropInstance = FItemInstance();
		return;
	}

	const FVector forward = GetActorForwardVector();
	const FVector spawnLocation = GetActorLocation() + forward * dropForwardDistance + FVector(0.f, 0.f, dropSpawnHeightOffset);
	const FTransform spawnTransform(GetActorRotation(), spawnLocation);

	ADropItem* drop = world->SpawnActorDeferred<ADropItem>(dropItemClass, spawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!drop)
	{
		pendingDropInstance = FItemInstance();
		return;
	}

	drop->InitializeItem(pendingDropInstance.itemId, pendingDropInstance.currentStackCount, pendingDropInstance.currentDurability);
	UGameplayStatics::FinishSpawningActor(drop, spawnTransform);

	drop->DropItem(forward);
	pendingDropInstance = FItemInstance();
}

void AWindlessCharacter::RegisterInteractable(AActor* interactable)
{
	if (!interactable)
		return;

	interactableTarget.AddUnique(interactable);
	UpdateInteractionTarget();
}

void AWindlessCharacter::UnregisterInteractable(AActor* interactable)
{
	interactableTarget.Remove(interactable);
	UpdateInteractionTarget();
}

void AWindlessCharacter::CollectSaveData(FInventorySaveRecord& outInventory, FPlayerStatSaveRecord& outStats) const
{
	if (inventory)
		inventory->SaveToRecord(outInventory);

	if (attributes)
		attributes->SaveToRecord(outStats);
}

void AWindlessCharacter::ApplySaveData(const FInventorySaveRecord& inventoryRecord, const FPlayerStatSaveRecord& statRecord)
{
	if (attributes)
		attributes->LoadFromRecord(statRecord);

	RefreshInventorySizeForLevel();

	if (inventory)
		inventory->LoadFromRecord(inventoryRecord);

	ApplyLevelToAllStats();

	if (bodyParts)
		bodyParts->HealAllParts();

	if (bTeleportToSavePointOnLoad)
		TeleportToLoadedSavePoint();

	RefreshQuickSlotHUD();
}

bool AWindlessCharacter::RequestSave(FName savePointId, FVector respawnLocation, FRotator respawnRotation)
{
	UGameInstance* gameInstance = GetGameInstance();
	if (!gameInstance)
		return false;

	UWindlessSaveSubsystem* saveSubsystem = gameInstance->GetSubsystem<UWindlessSaveSubsystem>();
	if (!saveSubsystem)
		return false;

	return saveSubsystem->SaveGameData(this, savePointId, respawnLocation, respawnRotation);
}

void AWindlessCharacter::CloseInventoryFromUI()
{
	if (APlayerController* controller = Cast<APlayerController>(GetController()))
		if (IsInventoryOpen())
			CloseInventory(controller);
}

void AWindlessCharacter::RefillRecoveryItem()
{
	if (inventory && attributes) // 시작 3, 5레벨마다 +1, 최대 10 (D-68, D-69 임시 값)
		inventory->RefillRecoveryItem(FMath::Min(10, 3 + attributes->GetCurrentLevel() / 5));
}

void AWindlessCharacter::RestAtSavePoint()
{
	if (attributes)
		attributes->ResetChaos();
	RefillRecoveryItem();

	UE_LOG(LogWindless, Log, TEXT("[세이브] 휴식 완료"));
}

void AWindlessCharacter::OpenStorage(AStorageContainer* container)
{
	if (!container)
		return;

	currentStorage = container;

	APlayerController* controller = Cast<APlayerController>(GetController());
	if (!controller)
		return;

	if (inventoryWidget && inventoryWidget->IsInViewport())
	{
		CloseInventory(controller);
		return;
	}

	OpenInventory(controller);

	if (inventoryWidget)
		inventoryWidget->SetStorageInventory(container->GetStorageInventory());
}

bool AWindlessCharacter::TryMolt()
{
	if (!attributes || !attributes->CanMolt())
		return false;

	if (!attributes->Molt())
		return false;

	ApplyLevelToAllStats();
	return true;
}

void AWindlessCharacter::ApplyLevelToAllStats()
{
	if (!attributes)
		return;

	const int32 level = attributes->GetCurrentLevel();
	attributes->ApplyStatsForLevel(level);

	if (bodyParts)
		bodyParts->ApplyLevelToParts(level);

	RefreshInventorySizeForLevel();
}

void AWindlessCharacter::RefreshInventorySizeForLevel()
{
	if (!attributes || !inventory)
		return;

	int32 newWidth = 0, newHeight = 0;
	WindlessLevelUp::GetInventorySizeForLevel(attributes->GetCurrentLevel(), newWidth, newHeight);

	if (newWidth != inventory->GetGridWidth() || newHeight != inventory->GetGridHeight())
	{
		inventory->ResizeGrid(newWidth, newHeight);
	}
}

FStaggerStatus AWindlessCharacter::GetStaggerStatus() const
{
	FStaggerStatus status;
	if (staggerComponent && staggerComponent->IsStrongStaggered())
	{
		status.state = EStaggerState::ESS_Strong;
		status.bInvincible = true;
	}
	else if (actionState == EActionState::EAS_HitReaction)
		status.state = EStaggerState::ESS_Weak;

	return status;
}

FStaggerStatus AWindlessCharacter::PlayerStagger() const
{
	return GetStaggerStatus();
}

float AWindlessCharacter::PlayerToughness() const
{
	return GetTenacity();
}

bool AWindlessCharacter::ForceStrongStagger(const FVector& fromLocation)
{
	if (!IsAlive() || (staggerComponent && staggerComponent->IsStrongStaggered()))
		return false;

	CancelCurrentAttack();
	BeginStrongStagger(fromLocation);
	return true;
}

void AWindlessCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (APlayerController* playerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			subsystem->AddMappingContext(WindlessContext, 0);
		}
	}

	Tags.Add(FName("EngagableTarget"));
	InitializeWindlessOverlay();
	InitializeInteractionText();
	BindCallbacks();
	if (inventory && inventory->GetQuickSlotInstanceUID(EQuickSlot::EQS_Recovery) == 0) // 처음 시작: 회복 아이템을 줌
		RefillRecoveryItem();
	if (weaponLoadout)
		weaponLoadout->InitializeLoadout();

	ApplyLevelToAllStats();
	InitializePostProcessingEffect();
	GetWorldTimerManager().SetTimerForNextTick(this, &AWindlessCharacter::HandleAutoLoad);
}

void AWindlessCharacter::Move(const FInputActionValue& value)
{
	if (IsOccupied() && actionState != EActionState::EAS_Blocking && actionState != EActionState::EAS_Aiming
		&& actionState != EActionState::EAS_UsingItem)
		return;

	FVector2D movementVector = value.Get<FVector2D>();
	currentMovementInput = movementVector;

	const FRotator controlRotation = Controller->GetControlRotation();
	const FRotator yawRotation(0.f, controlRotation.Yaw, 0.f);

	const FVector forward = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(forward, movementVector.Y);

	const FVector right = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(right, movementVector.X);
}

void AWindlessCharacter::StopMoving()
{
	currentMovementInput = FVector2D::ZeroVector;
}

void AWindlessCharacter::Look(const FInputActionValue& value)
{
	if (bQuickSlotActive) 
		return;

	const FVector2D lookAxisValue = value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(lookAxisValue.X);
		AddControllerPitchInput(lookAxisValue.Y);
	}
}

void AWindlessCharacter::Sprint()
{
	if (combat && combat->IsGuarding()) // 가드 중엔 달리기 금지
		return;

	if (bodyParts && !bodyParts->CanSprint()) // 다리 부위 부상 중엔 달리기 금지
		return;

	if (!attributes || attributes->GetStamina() <= 0.f) // 스태미나가 없으면 무시
		return;

	if (actionState != EActionState::EAS_Unoccupied) // 현재 캐릭터가 특정 상태일 때 무시 
		return;

	if (bIsMounted) // 현재 탈 것 탑승 중이면 무시 
		return;

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;
}

void AWindlessCharacter::HandleSprintStamina(float DeltaTime)
{
	if (!bIsSprinting || bIsMounted) // 캐릭터가 달리기 상태가 아니거나 탈 것 탑승 상태이면 무시 
		return;

	const bool bIsMoving = GetVelocity().SizeSquared2D() > 1.f;
	if (!bIsMoving) // 캐릭터의 속도가 1 미만일 때 무시
		return;

	if (attributes)
	{
		attributes->UseStamina(attributes->GetSprintStaminaCost() * DeltaTime);
		if (attributes->GetStamina() <= 0.f) // 스태미나가 없으면 달리기 중지
			StopSprinting();
	}
}

void AWindlessCharacter::StopSprinting()
{
	if (bIsMounted) // 현재 탈 것 탑승 중이면 무시 
		return;

	bIsSprinting = false;
	if (GetCharacterMovement())
		GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void AWindlessCharacter::ToggleCrouch()
{
	if (bIsDead || bIsMounted) // 사망 또는 탑승 중이면 무시
		return;

	const UCharacterMovementComponent* movement = GetCharacterMovement();
	const bool bCrouchRequested = bIsCrouched || (movement && movement->bWantsToCrouch);

	if (bCrouchRequested) // 앉기 해제는 다른 상태와 무관하게 항상 허용
	{
		UnCrouch();
		return;
	}

	if (!CanStartMovementAction(EMovementAction::EMA_Crouch))
		return;

	StopSprinting(); // 앉기 진입 시 달리기 해제
	Crouch();
}

void AWindlessCharacter::LockAirRotation()
{
	if (bAirRotationLocked || !bLockRotationInAir)
		return;

	UCharacterMovementComponent* movement = GetCharacterMovement();
	if (!movement)
		return;

	if (IsStrafing())
		return;

	cachedAirControl = movement->AirControl;
	movement->AirControl = airControlWhileLocked;
	bAirRotationLocked = true;
}

void AWindlessCharacter::UnlockAirRotation()
{
	if (!bAirRotationLocked)
		return;

	UCharacterMovementComponent* movement = GetCharacterMovement();
	if (!movement)
		return;

	movement->AirControl = cachedAirControl;
	bAirRotationLocked = false;
}

void AWindlessCharacter::HandleAirRotationLock()
{
	UCharacterMovementComponent* movement = GetCharacterMovement();
	if (!movement)
		return;

	const bool bInAir = movement->IsFalling();

	if (bInAir && !bAirRotationLocked && bLockRotationOnFall)
	{
		LockAirRotation();
		return;
	}

	if (!bInAir && bAirRotationLocked)
		UnlockAirRotation();
}

void AWindlessCharacter::RefreshRotationMode()
{
	UCharacterMovementComponent* movement = GetCharacterMovement();
	if (!movement)
		return;

	if (actionState == EActionState::EAS_Knockdown) // 강한 경직 중 회전 무시
	{
		movement->bUseControllerDesiredRotation = false;
		movement->bOrientRotationToMovement = false;
		return;
	}

	const bool bStrafe = IsStrafing();
	const bool bAttackTurning = attackTurnRemaining > 0.f;
	movement->bUseControllerDesiredRotation = bStrafe && !bAttackTurning;
	movement->bOrientRotationToMovement = !bStrafe && !bAirRotationLocked;
}

void AWindlessCharacter::Attack()
{
	if (combat)
		combat->HandleAttackPressed(EAttackHand::Left, EAttackType::EAT_Light);
}

void AWindlessCharacter::AttackEnd()
{
	if (aiSignals) 
		aiSignals->NotifyAttackEnd();

	if (combat)
		combat->ResetCombo();

	if (actionState != EActionState::EAS_Attacking)
		return;

	actionState = EActionState::EAS_Unoccupied;
	ApplyActionDelay();
}

void AWindlessCharacter::RightAttack()
{
	if (quickSlots && quickSlots->HandleCancelInput())
		return;

	if (combat)
		combat->HandleAttackPressed(EAttackHand::Right, EAttackType::EAT_Light);
}

void AWindlessCharacter::LeftStrongAttack()
{
	if (combat)
		combat->HandleAttackPressed(EAttackHand::Left, EAttackType::EAT_Heavy);
}

void AWindlessCharacter::RightStrongAttack()
{
	if (combat)
		combat->HandleAttackPressed(EAttackHand::Right, EAttackType::EAT_Heavy);
}

void AWindlessCharacter::PlayAttackMontageSingle(UAnimMontage* montage, bool bStrong)
{
	if (!montage) // 애니메이션 몽타주가 없으면 무시 
		return;

	if (UAnimInstance* anim = GetMesh()->GetAnimInstance())
	{
		float rate = bodyParts ? bodyParts->GetAttackRateMultiplier() : 1.f;
		if (bStrong && bodyParts)
			rate /= bodyParts->GetStrongAttackTimeMultiplier();

		if (attributes)
			rate *= attributes->GetAttackRateMultiplier();

		anim->Montage_Play(montage, rate);
	}
}

void AWindlessCharacter::StopGuard()
{
	if (combat)
		combat->StopGuard();
}

void AWindlessCharacter::LeftAttackHeld()
{
	if (combat)
		combat->HandleAttackHeld(EAttackHand::Left);
}

void AWindlessCharacter::RightAttackHeld()
{
	if (combat)
		combat->HandleAttackHeld(EAttackHand::Right);
}

void AWindlessCharacter::LeftAttackReleased()
{
	if (combat)
		combat->HandleAttackReleased(EAttackHand::Left);
}

void AWindlessCharacter::RightAttackReleased()
{
	if (combat)
		combat->HandleAttackReleased(EAttackHand::Right);
}

void AWindlessCharacter::UpdateAttackFacing(float DeltaTime)
{
	if (attackTurnRemaining <= 0.f)
		return;

	if (actionState != EActionState::EAS_Attacking) // 경직/회피 등으로 공격이 끊기면 중단
	{
		attackTurnRemaining = 0.f;
		return;
	}

	attackTurnRemaining -= DeltaTime;
	const FRotator targetRotation(0.f, attackFacingYaw, 0.f);

	if (attackTurnRemaining <= 0.f)
		SetActorRotation(targetRotation);
	else
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), targetRotation, DeltaTime, attackTurnSpeed));
}

void AWindlessCharacter::HitReactEnd()
{
	if (actionState == EActionState::EAS_HitReaction)
		actionState = EActionState::EAS_Unoccupied;
}

void AWindlessCharacter::OnStaggered(const FVector& impactPoint, AActor* hitter)
{
	if (bow)
		bow->CancelDraw(TEXT("경직"));

	if (actionState == EActionState::EAS_Dodge)
		ClearDodgeState();

	CancelItemCast(true);
	if (quickSlots && GetLastStaggerResult() == EStaggerResult::ESR_Strong) // 강한 경직만 부위 선택을 취소(D-49)
		quickSlots->ForceCancel();

	Super::OnStaggered(impactPoint, hitter);

	if (!IsAlive())
		return;

	if (GetLastStaggerResult() == EStaggerResult::ESR_Strong)
	{
		BeginStrongStagger(hitter ? hitter->GetActorLocation() : impactPoint);
		return;
	}

	actionState = EActionState::EAS_HitReaction;
}

void AWindlessCharacter::OnEndured()
{
	Super::OnEndured();
	UE_LOG(LogWindless, Log, TEXT("[경직] 버팀 - 현재 행동 유지"));
}

void AWindlessCharacter::CancelCurrentAttack()
{
	if (aiSignals) 
		aiSignals->NotifyAttackEnd();

	if (combat)
		combat->ResetCombo();

	Super::CancelCurrentAttack();

	if (actionState == EActionState::EAS_Attacking)
		actionState = EActionState::EAS_Unoccupied;

	currentAttackWeapon = nullptr;
}

void AWindlessCharacter::ApplyGuardBreakStagger()
{
	UE_LOG(LogWindless, Warning, TEXT("[경직] 가드 브레이크 - 무방비 상태"));
	SetForceStaggerOnHit(true);
}

void AWindlessCharacter::BeginStrongStagger(const FVector& fromLocation)
{
	if (!staggerComponent)
	{
		actionState = EActionState::EAS_HitReaction;
		return;
	}

	if (combat && combat->IsGuarding())
		StopGuard();

	actionState = EActionState::EAS_Knockdown;
	staggerComponent->BeginStrongStagger(fromLocation);
}

void AWindlessCharacter::HandleStrongStaggerEnded()
{
	if (actionState == EActionState::EAS_Knockdown)
		actionState = EActionState::EAS_Unoccupied;
}

void AWindlessCharacter::Die_Implementation()
{
	if (bIsDead)
		return;

	bIsDead = true;
	if (staggerComponent)
		staggerComponent->CancelStagger();

	Super::Die_Implementation();
	ClearDodgeState();
	actionState = EActionState::EAS_Dead;
	DisableMeshCollision();

	if (quickSlots)
		quickSlots->ForceCancel();
	if (bodyParts)
		bodyParts->DisableAllParts();
	if (bow)
		bow->CancelDraw(TEXT("사망"));
	if (targetingComponent)
		targetingComponent->ReleaseLockOn(TEXT("플레이어 사망"));

	onPlayerDeath.Broadcast();
}

void AWindlessCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ClearDodgeState();
	SetAnimRootMotionTranslationScale(1.0f);

	if (actionState == EActionState::EAS_Dodge) // 구르기 상태일 때만 해제
	{
		actionState = EActionState::EAS_Unoccupied;
		ApplyActionDelay();
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage)
			AnimInstance->Montage_SetPlayRate(CurrentMontage, dodgePlayRate);
	}
}

void AWindlessCharacter::OnSpacePressed()
{
	if (bIsMounted) // 탑승 상태이면 구르기 무시
		return;


	if (staggerComponent && staggerComponent->IsStrongStaggered()) // 강한 경직 중 다운 시 구르기 기상
	{
		if (staggerComponent->GetPhase() != EStrongStaggerPhase::ESSP_Down)
			return;

		if (!HasEnoughDodgeStamina())
		{
			UE_LOG(LogWindless, Log, TEXT("[경직] 구르며 기상 불가(스태미나 부족)"));
			return;
		}

		staggerComponent->TryRollGetUp();
	}

	if (!currentMovementInput.IsNearlyZero()) // 이동 입력이 있으면 해당 이동 방향으로 구르기
		Dodge();

	else // 이동 입력이 없으면 뒷방향으로 구르기
	{
		currentMovementInput = FVector2D(0.f, -1.f);
		Dodge();
		currentMovementInput = FVector2D::ZeroVector;
	}
}

void AWindlessCharacter::Dodge()
{
	if (!attributes)
		return;

	if (!HasEnoughDodgeStamina())
	{
		UE_LOG(LogWindless, Log, TEXT("[회피불가] 스태미나 부족 (필요 %.1f / 현재 %.1f)"), attributes->GetDodgeCost(), GetCurrentStamina());
		return;
	}

	if (CheckActionDelay())
		return;

	if (!CanStartMovementAction(EMovementAction::EMA_Dodge, true)) // 가드/차징을 풀기 전에 회피 가능 여부 확인
		return;

	if (combat && combat->IsGuarding()) // 가드 중 회피는 가드를 해제하고 진행
		StopGuard();

	if (bow && bow->IsDrawing()) // 활 차징 중 회피는 차징을 취소하고 진행
		bow->CancelDraw(TEXT("회피"));

	if (IsCrouchRequested())
		UnCrouch();

	const bool bStep = CanUseLockOnStep();
	if (bStep)
	{
		FaceLockOnTarget();
		PlayStepDodgeMontage(GetStepDodgeSection());
	}
	else
	{
		RotateToMovementInputDirection();
		PlayDodgeMontage();
	}

	const float legDistanceScale = bodyParts ? bodyParts->GetDodgeDistanceMultiplier() : 1.f;
	const float stepDistanceScale = bStep ? stepDodgeDistanceScale : 1.f;
	SetAnimRootMotionTranslationScale(legDistanceScale * stepDistanceScale);

	actionState = EActionState::EAS_Dodge;
	bPerfectDodgeConsumed = false;
	ConsumeStamina(attributes->GetDodgeCost());

	UE_LOG(LogWindless, Log, TEXT("[회피] %s 스태미나 -%.0f"),
		bStep ? *FString::Printf(TEXT("락온 스텝(%s)"), *GetStepDodgeSection().ToString()) : TEXT("구르기"),
		attributes->GetDodgeCost());
}

void AWindlessCharacter::RotateToMovementInputDirection()
{
	if (!currentMovementInput.IsNearlyZero() && Controller)
	{
		const FRotator controlRotation = Controller->GetControlRotation();
		const FRotator yawRotation(0.f, controlRotation.Yaw, 0.f);

		const FVector forward = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
		const FVector right = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
		const FVector dodgeDirection = (forward * currentMovementInput.Y + right * currentMovementInput.X).GetSafeNormal();

		if (!dodgeDirection.IsNearlyZero())
		{
			const FRotator newRotation = dodgeDirection.Rotation();
			SetActorRotation(FRotator(0.f, newRotation.Yaw, 0.f));
		}
	}
}

bool AWindlessCharacter::HasEnoughDodgeStamina()
{
	return HasStamina(attributes->GetDodgeCost());
}

bool AWindlessCharacter::CanUseLockOnStep() const
{
	return stepDodgeMontage && targetingComponent && targetingComponent->IsLockedOn() && targetingComponent->GetCurrentTarget();
}

void AWindlessCharacter::FaceLockOnTarget()
{
	const AActor* target = targetingComponent ? targetingComponent->GetCurrentTarget() : nullptr;
	if (!target)
		return;

	const FVector toTarget = (target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (!toTarget.IsNearlyZero())
		SetActorRotation(FRotator(0.f, toTarget.Rotation().Yaw, 0.f));
}


FName AWindlessCharacter::GetStepDodgeSection() const
{
	const float inputX = currentMovementInput.X;
	const float inputY = currentMovementInput.Y;

	if (FMath::IsNearlyZero(inputX) && FMath::IsNearlyZero(inputY))
		return stepBackwardSection;

	if (FMath::Abs(inputY) >= FMath::Abs(inputX))
		return inputY >= 0.f ? stepForwardSection : stepBackwardSection;

	return inputX >= 0.f ? stepRightSection : stepLeftSection;
}

void AWindlessCharacter::PlayStepDodgeMontage(FName sectionName)
{
	UAnimInstance* anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!anim || !stepDodgeMontage)
		return;

	anim->Montage_Play(stepDodgeMontage);
	anim->Montage_JumpToSection(sectionName, stepDodgeMontage);
}

void AWindlessCharacter::ClearDodgeState()
{
	SetInvincible(false);
	SetAnimRootMotionTranslationScale(1.0f);
}

void AWindlessCharacter::GetUpStart()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage();
		if (CurrentMontage)
			AnimInstance->Montage_SetPlayRate(CurrentMontage, getUpPlayRate);
	}
}

void AWindlessCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (aiSignals) 
		aiSignals->NotifyAttackActive(CollisionEnabled != ECollisionEnabled::NoCollision);

	if (currentAttackWeapon && currentAttackWeapon->GetWeaponBox())
	{
		currentAttackWeapon->ignoreActors.Empty();
		currentAttackWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
	}
}

void AWindlessCharacter::EKeyPressed()
{
	if (bQuickSlotActive)
		return;

	if (bIsMounted)
		return;

	if (CanDisarm())
		Disarm();
	else if (CanArm())
		Arm();
}

void AWindlessCharacter::PlayEquipMontage(const FName& sectionName)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && equipMontage)
	{
		animInstance->Montage_Play(equipMontage);
		animInstance->Montage_JumpToSection(sectionName, equipMontage);
	}
}

bool AWindlessCharacter::CanDisarm()
{
	return actionState == EActionState::EAS_Unoccupied &&
		characterState != ECharacterState::ECS_Unequipped;
}

bool AWindlessCharacter::CanArm()
{
	return actionState == EActionState::EAS_Unoccupied &&
		characterState == ECharacterState::ECS_Unequipped &&
		rightHandWeapon;
}

void AWindlessCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	characterState = ECharacterState::ECS_Unequipped;
	actionState = EActionState::EAS_EquippingWeapon;
}

void AWindlessCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	characterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	actionState = EActionState::EAS_EquippingWeapon;
}

void AWindlessCharacter::AttachWeaponToBack()
{
	if (rightHandWeapon)
		rightHandWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
}

void AWindlessCharacter::AttachWeaponToHand()
{
	if (!rightHandWeapon)
		return;

	const FName socketName = rightHandWeapon->GetHandSocketOverride().IsNone()
		? FName("RightHandSocket") : rightHandWeapon->GetHandSocketOverride();
	rightHandWeapon->AttachMeshToSocket(GetMesh(), socketName);
}

void AWindlessCharacter::FinishEquipping()
{
	actionState = EActionState::EAS_Unoccupied;
}

void AWindlessCharacter::OnLockOnPressed()
{
	if (bQuickSlotActive)
		return;

	if (attributes && attributes->IsLockOnInputBlocked())
		return;

	if (IsBowEquipped())
		return;

	if (targetingComponent)
		targetingComponent->ToggleLockOn();
}

void AWindlessCharacter::OnSwitchTargetTriggered(const FInputActionValue& Value)
{
	if (!targetingComponent || !targetingComponent->IsLockedOn()) 
		return;

	const float Axis = Value.Get<float>();
	if (FMath::Abs(Axis) < 0.5f) // 입력 값이 약한 경우 무시
		return; 

	targetingComponent->SwitchTarget(Axis);
}

void AWindlessCharacter::ToggleMount()
{
	if (bQuickSlotActive)
		return;

	if (!mountComponent)
		return;

	if (IsMounted())
	{
		mountComponent->Dismount();
		return;
	}

	if (IsOccupied())
		return;

	mountComponent->Mount();
}

void AWindlessCharacter::HandleMountPhaseChanged(EMountPhase phase)
{
	switch (phase)
	{
		case EMountPhase::EMP_Mounting: // 탈 것 소환 중
			actionState = EActionState::EAS_Mounting;
			break;

		case EMountPhase::EMP_Mounted: // 탈 것 탑승 완료
			bIsMounted = true;
			actionState = EActionState::EAS_Unoccupied;
			break;

		case EMountPhase::EMP_Dismounting: // 탈 것 탑승 해제 완료
			bIsMounted = false;
			actionState = EActionState::EAS_Dismounting;
			if (GetCharacterMovement())
				GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

			break;

		case EMountPhase::EMP_Grounded: // 탈 것 소멸 중
			actionState = EActionState::EAS_Unoccupied;
			break;
	}
}

void AWindlessCharacter::HandleChaos(float DeltaTime)
{
	if (!attributes)
		return;

	float zoneLimit = 0.f;
	const EChaosZone zone = GetChaosZone(zoneLimit);
	attributes->TickChaos(DeltaTime, zone, zoneLimit);
	ForceUnlockOnCollapseState(DeltaTime);
	HandleLockOnCameraNoise(DeltaTime);
	SetHUDChaos();
}

void AWindlessCharacter::EnterLightZone()
{
	++lightZoneCount;
}

void AWindlessCharacter::ExitLightZone()
{
	lightZoneCount = FMath::Max(0, lightZoneCount - 1);
}

void AWindlessCharacter::AddChaosZone(const AChaosZoneVolume* zone)
{
	chaosZones.AddUnique(zone);
}

void AWindlessCharacter::RemoveChaosZone(const AChaosZoneVolume* zone)
{
	chaosZones.Remove(zone);
}

EChaosZone AWindlessCharacter::GetChaosZone(float& outLimit) const
{
	// 빛과 어둠이 겹치면 빛이 이긴다. 빛끼리는 가장 낮은 하한, 어둠끼리는 가장 높은 상한을 쓴다
	bool bLight = lightZoneCount > 0, bDark = false;
	float lightFloor = attributes ? attributes->GetLightZoneChaosFloor() : 0.f, darkCap = 0.f;
	for (const TWeakObjectPtr<const AChaosZoneVolume>& zone : chaosZones)
	{
		if (!zone.IsValid())
			continue;

		if (zone->GetZone() == EChaosZone::Light)
		{
			lightFloor = bLight ? FMath::Min(lightFloor, zone->GetLimit()) : zone->GetLimit();
			bLight = true;
		}
		else
		{
			darkCap = bDark ? FMath::Max(darkCap, zone->GetLimit()) : zone->GetLimit();
			bDark = true;
		}
	}

	outLimit = bLight ? lightFloor : darkCap;
	return bLight ? EChaosZone::Light : bDark ? EChaosZone::Dark : EChaosZone::Shade;
}

bool AWindlessCharacter::IsInLightZone() const
{
	float limit = 0.f;
	return GetChaosZone(limit) == EChaosZone::Light;
}

void AWindlessCharacter::ResetChaosGauge()
{
	if (attributes)
		attributes->ResetChaos();
}

void AWindlessCharacter::NotifyPerfectDodge()
{
	if (attributes)
		attributes->OnPerfectDodge();
}

void AWindlessCharacter::HandleChaosStateChanged(EChaosState newState)
{
	UE_LOG(LogWindless, Warning, TEXT("[혼돈] 단계 변경 -> %d (현재 혼돈 %.1f)"), (int32)newState, attributes ? attributes->GetChaos() : 0.f);
	DisableLockOn(newState);
	SetPostProcessingEffect(newState);
	UpdateChaosWobbleShake(newState);
	if (windlessOverlay)
		windlessOverlay->SetChaosBarColor(newState);

	OnChaosStateChangedBP(newState);
}

void AWindlessCharacter::ToggleInventory()
{
	if (bQuickSlotActive)
		return;

	APlayerController* controller = Cast<APlayerController>(GetController());
	if (!controller)
		return;

	if (inventoryWidget && inventoryWidget->IsInViewport())
		CloseInventory(controller);
	else
		OpenInventory(controller);
}

void AWindlessCharacter::FinishItemCast()
{
	FName usedItemId = NAME_None;

	if (inventory && castingItemUID != 0)
	{
		const FItemInstance inst = inventory->FindInstance(castingItemUID);
		usedItemId = inst.itemId;

		const bool bUsed = (castingTargetPart == EBodyPart::EBP_None)
			? inventory->UseItem(castingItemUID)
			: inventory->UseItemOnPart(castingItemUID, castingTargetPart);

		if (bUsed && quickSlots)
			quickSlots->NotifyItemUsed(usedItemId);
	}

	castingItemUID = 0;
	castingTargetPart = EBodyPart::EBP_None;

	if (itemCastRecoveryTime > 0.f && actionState == EActionState::EAS_UsingItem)
	{
		GetWorldTimerManager().SetTimer(itemCastRecoveryTimer, this, &AWindlessCharacter::EndItemCastRecovery, itemCastRecoveryTime, false);
		UE_LOG(LogWindless, Log, TEXT("[시전] 완료 - 후딜 %.2f초"), itemCastRecoveryTime);
		return;
	}

	EndItemCastRecovery();
}

void AWindlessCharacter::ApplyCastMovementRestriction(float speedRate)
{
	UCharacterMovementComponent* movement = GetCharacterMovement();
	if (!movement)
		return;

	castCachedWalkSpeed = movement->MaxWalkSpeed;
	bIsSprinting = false;
	movement->MaxWalkSpeed = walkSpeed * FMath::Max(0.f, speedRate);
}

void AWindlessCharacter::EndItemCastRecovery()
{
	GetWorldTimerManager().ClearTimer(itemCastRecoveryTimer);
	ClearCastMovementRestriction();

	if (actionState == EActionState::EAS_UsingItem)
	{
		actionState = EActionState::EAS_Unoccupied;
		UE_LOG(LogWindless, Log, TEXT("[시전] 후딜 종료 - 행동 가능"));
	}
	if (aiSignals) 
		aiSignals->NotifyItemUse(false);
}

void AWindlessCharacter::ClearCastMovementRestriction()
{
	if (UCharacterMovementComponent* movement = GetCharacterMovement())
		movement->MaxWalkSpeed = walkSpeed;

	castCachedWalkSpeed = 0.f;
}

void AWindlessCharacter::HandleItemDropped(FItemInstance droppedInstance)
{
	pendingDropInstance = droppedInstance;
	UAnimInstance* animInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (dropMontage && animInstance && !bIsMounted)
	{
		animInstance->Montage_Play(dropMontage);
		GetWorldTimerManager().SetTimer(dropFallbackTimer, this, &AWindlessCharacter::SpawnDroppedItem, dropFallbackDelay, false);
		return;
	}

	SpawnDroppedItem();
}

void AWindlessCharacter::Interact()
{
	if (bQuickSlotActive)
		return;

	if (dialogueComponent && dialogueComponent->IsDialogueActive())
		return;

	if (bIsMounted)
		return;

	if (!currentInteractable || !currentInteractable->Implements<UInteractableInterface>())
		return;

	if (!IInteractableInterface::Execute_CanInteract(currentInteractable, this))
		return;

	UE_LOG(LogWindless, Log, TEXT("상호작용 실행 대상 = %s"), *currentInteractable->GetName());
	IInteractableInterface::Execute_Interact(currentInteractable, this);
	UpdateInteractionTarget();
}

void AWindlessCharacter::UpdateInteractionTarget()
{
	const bool bInteractionBlocked =
		(dialogueComponent && dialogueComponent->IsDialogueActive()) ||
		(maintenance && maintenance->IsInMaintenance());

	if (bInteractionBlocked)
	{
		currentInteractable = nullptr;
		if (interactionWidget)
			interactionWidget->SetVisibility(ESlateVisibility::Collapsed);
		SetInteractionInputMode(false);
		return;
	}

	AActor* nearest = nullptr;
	float nearestDistSq = TNumericLimits<float>::Max();
	const FVector myLocation = GetActorLocation();

	for (int32 i = interactableTarget.Num() - 1; i >= 0; --i)
	{
		AActor* candidate = interactableTarget[i];
		if (!IsValid(candidate) || !candidate->Implements<UInteractableInterface>())
		{
			interactableTarget.RemoveAt(i);
			continue;
		}

		if (!IInteractableInterface::Execute_CanInteract(candidate, this))
			continue;

		const float distSq = FVector::DistSquared(myLocation, candidate->GetActorLocation());
		if (distSq < nearestDistSq)
		{
			nearestDistSq = distSq;
			nearest = candidate;
		}
	}

	currentInteractable = nearest;
	SetInteractionWidgetText();
}

void AWindlessCharacter::InitializeInteractionText()
{
	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (!playerController || !interactionTargetClass)
		return;

	interactionWidget = CreateWidget<UInteractionWidget>(playerController, interactionTargetClass);
	if (interactionWidget)
	{
		interactionWidget->AddToViewport();
		interactionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AWindlessCharacter::HandleAutoLoad()
{
	if (!bAutoLoadOnStart)
		return;

	UGameInstance* gameInstance = GetGameInstance();
	if (!gameInstance)
		return;

	if (UWindlessSaveSubsystem* saveSubsystem = gameInstance->GetSubsystem<UWindlessSaveSubsystem>())
		saveSubsystem->TryContinue(this);
}

void AWindlessCharacter::TeleportToLoadedSavePoint()
{
	UGameInstance* gameInstance = GetGameInstance();
	if (!gameInstance)
		return;

	UWindlessSaveSubsystem* saveSubsystem = gameInstance->GetSubsystem<UWindlessSaveSubsystem>();
	if (!saveSubsystem)
		return;

	FVector loc = FVector::ZeroVector;
	FRotator rot = FRotator::ZeroRotator;
	if (!saveSubsystem->GetLoadedRespawnTransform(loc, rot))
		return;

	SetActorLocationAndRotation(loc, rot, false, nullptr, ETeleportType::TeleportPhysics);
}

void AWindlessCharacter::HandleQuickSlotHighlightChanged(EQuickSlot quickSlot)
{
	if (windlessOverlay)
		windlessOverlay->SetSelectedQuickSlot(quickSlot);
}

void AWindlessCharacter::HandleMaintenanceStateChanged(bool bInMaintenance)
{
	UpdateInteractionTarget();

	if (windlessOverlay)
		windlessOverlay->SetRenderOpacity(bInMaintenance ? 0.f : 1.f);
}

void AWindlessCharacter::HandleMoltingChanged(int32 newLevel, float expPercent)
{
	if (windlessOverlay && attributes)
		windlessOverlay->SetMoltStones(attributes->GetMoltStones());
}

bool AWindlessCharacter::CanChangeEquipSlot(EEquipSlot slot) const
{
	if (slot != EEquipSlot::EES_LeftHand && slot != EEquipSlot::EES_RightHand)
		return true;

	FString reason;
	if (IsInCombatState())
		reason = TEXT("전투 중에는 무기를 교체할 수 없습니다");
	else if (actionState != EActionState::EAS_Unoccupied)
		reason = TEXT("행동 중에는 무기를 교체할 수 없습니다");

	if (reason.IsEmpty())
		return true;

	UE_LOG(LogWindless, Log, TEXT("[무기교체불가] %s"), *reason);
	return false;
}

void AWindlessCharacter::RefreshCharacterState()
{
	if (!leftHandWeapon && !rightHandWeapon)
	{
		characterState = ECharacterState::ECS_Unequipped;
		return;
	}

const bool bTwoHanded = (rightHandWeapon && rightHandWeapon->OccupiesBothHands()) || (leftHandWeapon && leftHandWeapon->OccupiesBothHands()); 
	characterState = bTwoHanded ? ECharacterState::ECS_EquippedTwoHandedWeapon : ECharacterState::ECS_EquippedOneHandedWeapon;
}

void AWindlessCharacter::HandleStamina(float DeltaTime)
{
	if (!attributes || !windlessOverlay)
		return;

	HandleSprintStamina(DeltaTime);
	if (!bIsSprinting && !IsGuarding())
	{
		const float regenMul = bodyParts ? bodyParts->GetStaminaRegenMultiplier() : 1.f;
		attributes->RegenStamina(DeltaTime, regenMul);
	}
	SetHUDStamina();
}

void AWindlessCharacter::SetPostProcessingEffect(EChaosState newState)
{
	if (vignetteMID)
	{
		float vignette = 0.f;
		switch (newState)
		{
		case EChaosState::ECS_Collapse2:
			vignette = 0.75f;
			break;
		case EChaosState::ECS_Collapse3:
			vignette = 1.f;
			break;
		default:
			vignette = 0.f;
			break;
		}
		vignetteMID->SetScalarParameterValue(TEXT("VignetteStrength"), vignette);
	}
}

void AWindlessCharacter::ForceUnlockOnCollapseState(float DeltaTime)
{
	if (!attributes)
		return;

	if (attributes->GetChaosState() == EChaosState::ECS_Collapse2 &&
		targetingComponent && targetingComponent->IsLockedOn())
	{
		collapse2UnlockTimer += DeltaTime;
		if (collapse2UnlockTimer >= attributes->GetLockOnBreakInterval())
		{
			collapse2UnlockTimer = 0.f;
			if (FMath::FRandRange(0.f, 100.f) <= attributes->GetLockOnBreakChance())
			{
				targetingComponent->ReleaseLockOn(TEXT("붕괴2 강제 해제"));
				if (lockOnForcedBreakSound)
					UGameplayStatics::PlaySound2D(this, lockOnForcedBreakSound);
			}
		}
	}
	else
		collapse2UnlockTimer = 0.f;
}

void AWindlessCharacter::DisableLockOn(EChaosState newState)
{
	if (newState == EChaosState::ECS_Collapse3 &&
		targetingComponent && targetingComponent->IsLockedOn())
	{
		targetingComponent->ReleaseLockOn(TEXT("붕괴3 락온 차단"));
	}
}

void AWindlessCharacter::HandleLockOnCameraNoise(float DeltaTime)
{
	const float interval = attributes ? attributes->GetLockOnShakeInterval() : 0.f;
	const bool bLocked = targetingComponent && targetingComponent->IsLockedOn();

	if (interval <= 0.f || !bLocked || !lockOnNoiseShakeClass)
	{
		lockOnShakeTimer = 0.f;
		return;
	}

	lockOnShakeTimer += DeltaTime;
	if (lockOnShakeTimer < interval)
		return;

	lockOnShakeTimer = 0.f;
	if (APlayerController* playerController = Cast<APlayerController>(GetController()))
	{
		playerController->ClientStartCameraShake(lockOnNoiseShakeClass);
		UE_LOG(LogWindless, Log, TEXT("[락온] 카메라 노이즈 (주기 %.1f초)"), interval);
	}
}

void AWindlessCharacter::UpdateChaosWobbleShake(EChaosState newState)
{
	const APlayerController* playerController = Cast<APlayerController>(GetController());
	APlayerCameraManager* cameraManager = playerController ? playerController->PlayerCameraManager.Get() : nullptr;
	if (!cameraManager || !chaosWobbleShakeClass)
		return;

	const bool bShouldWobble = (newState == EChaosState::ECS_Collapse3);
	if (bShouldWobble == bChaosWobbleActive)
		return;

	if (bShouldWobble)
		cameraManager->StartCameraShake(chaosWobbleShakeClass, chaosWobbleShakeScale);
	else
		cameraManager->StopAllInstancesOfCameraShake(chaosWobbleShakeClass, false);

	bChaosWobbleActive = bShouldWobble;
	UE_LOG(LogWindless, Log, TEXT("[혼돈] 붕괴3 화면 일렁임 %s"), bShouldWobble ? TEXT("시작") : TEXT("종료"));
}

void AWindlessCharacter::HandleBodyRegen(float DeltaTime)
{
	if (bodyParts)
		bodyParts->RegenHealth(DeltaTime);
}

void AWindlessCharacter::HandleLegFractureInjury(float DeltaTime)
{
	if (!bodyParts || bIsMounted)
		return;

	if (bodyParts->IsCrawling())
		GetCharacterMovement()->MaxWalkSpeed = crawlSpeed;

	const float moveCost = bodyParts->GetMoveHpCost();
	if (moveCost <= 0.f)
		return;

	const bool bMoving = GetVelocity().SizeSquared2D() > 1.f;
	const bool bDodging = (actionState == EActionState::EAS_Dodge);
	if (bMoving || bDodging)
	{
		bodyParts->ApplyFlatDamage(EBodyPart::EBP_LeftLeg, moveCost * DeltaTime);
		bodyParts->ApplyFlatDamage(EBodyPart::EBP_RightLeg, moveCost * DeltaTime);
	}
}

void AWindlessCharacter::ToggleBodyStatus()
{
	if (bQuickSlotActive)
		return;

	if (!windlessOverlay)
		return;

	windlessOverlay->ToggleBodyStatus();
	if (windlessOverlay->IsBodyStatusVisible() && bodyParts)
		bodyParts->BroadcastAllParts();
}

void AWindlessCharacter::HandleBodyPartChanged(EBodyPart part, float percent, bool bInjured, bool bDisabled)
{
	UpdateBodyPartOverlay(part, percent, bInjured, bDisabled);
	HandleArmFractureInjury(bDisabled, part);
	HandleEyeDamageInjury();

	if (!IsAlive() && !ActorHasTag(FName("Dead")))
		Die();
}

void AWindlessCharacter::UpdateBodyPartOverlay(EBodyPart part, float percent, bool bInjured, bool bDisabled)
{
	if (windlessOverlay)
	{
		windlessOverlay->UpdateBodyPart(part, percent, bInjured, bDisabled);
		if (bodyParts)
			windlessOverlay->SetHealthBarPercent(bodyParts->GetTotalHealthPercent());
	}
}

void AWindlessCharacter::HandleArmFractureInjury(bool bDisabled, EBodyPart part)
{
	if (bDisabled && (part == EBodyPart::EBP_LeftArm || part == EBodyPart::EBP_RightArm))
	{
		const bool bLeftBroken = (part == EBodyPart::EBP_LeftArm);
		if (IsGuarding() && IsGuardOnLeftHand() == bLeftBroken)
			StopGuard();
		if (bow)
			bow->CancelDraw(TEXT("팔 파괴"));
	}
}

void AWindlessCharacter::HandleEyeDamageInjury()
{
	if (bodyParts)
	{
		const bool bEye = bodyParts->HasEyeDamage();
		if (bEye != bEyeDamageActive)
		{
			bEyeDamageActive = bEye;
			OnEyeDamageChanged(bEye);
			UE_LOG(LogWindless, Warning, TEXT("[안구손상] %s -> 락온거리배율 %.2f"), bEye ? TEXT("발동") : TEXT("회복"), GetLockOnRangeMultiplier());
		}
	}
}

void AWindlessCharacter::OpenInventory(APlayerController* controller)
{
	if (!inventoryWidgetClass)
		return;

	if (inventoryWidget && inventoryWidget->IsInViewport())
		return;

	inventoryWidget = CreateWidget<UInventoryWidget>(controller, inventoryWidgetClass);
	if (!inventoryWidget)
		return;

	inventoryWidget->AddToViewport();
	SetInventoryInputMode(true);

	FInputModeGameAndUI uiMode;
	uiMode.SetWidgetToFocus(inventoryWidget->TakeWidget());
	controller->SetInputMode(uiMode);
	controller->bShowMouseCursor = true;
}

void AWindlessCharacter::CloseInventory(APlayerController* controller)
{
	if (inventoryWidget)
		inventoryWidget->SetStorageInventory(nullptr);

	currentStorage = nullptr;

	if (inventoryWidget)
		inventoryWidget->RemoveFromParent();

	inventoryWidget = nullptr;
	SetInventoryInputMode(false);

	controller->SetInputMode(FInputModeGameOnly());
	controller->bShowMouseCursor = false;
}

void AWindlessCharacter::SetInteractionWidgetText()
{
	if (currentInteractable)
	{
		if (interactionWidget)
		{
			const FText Text = IInteractableInterface::Execute_GetInteractionText(currentInteractable);
			interactionWidget->SetActionText(Text);
			interactionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		SetInteractionInputMode(true);
	}
	else
	{
		if (interactionWidget)
			interactionWidget->SetVisibility(ESlateVisibility::Collapsed);
		SetInteractionInputMode(false);
	}
}

void AWindlessCharacter::RefreshQuickSlotHUD()
{
	if (!windlessOverlay || !inventory)
		return;

	for (uint8 i = (uint8)EQuickSlot::EQS_Recovery; i < (uint8)EQuickSlot::EQS_MAX; ++i)
	{
		const EQuickSlot quickSlot = (EQuickSlot)i;
		const int64 uid = inventory->GetQuickSlotInstanceUID(quickSlot);

		if (uid == 0)
		{
			windlessOverlay->UpdateQuickSlot(quickSlot, nullptr, 0);
			continue;
		}

		const FItemInstance inst = inventory->FindInstance(uid);
		FItemData data;
		if (!inventory->GetItemData(inst.itemId, data))
			continue;

		windlessOverlay->UpdateQuickSlot(quickSlot, data.icon, inst.currentStackCount);
	}
}

void AWindlessCharacter::RefreshQuickSlotCooldownUI()
{
	if (!windlessOverlay || !quickSlots)
		return;

	for (uint8 i = (uint8)EQuickSlot::EQS_Recovery; i < (uint8)EQuickSlot::EQS_MAX; ++i)
	{
		const EQuickSlot quickSlot = (EQuickSlot)i;
		windlessOverlay->SetQuickSlotCooldown(quickSlot, quickSlots->GetSlotCooldownRatio(quickSlot));
	}
}

void AWindlessCharacter::PressQuickSlotRecovery()
{
	if (quickSlots)
		quickSlots->HandleSlotPressed(EQuickSlot::EQS_Recovery);
}

void AWindlessCharacter::PressQuickSlotConsume1()
{
	if (quickSlots)
		quickSlots->HandleSlotPressed(EQuickSlot::EQS_Consume1);
}

void AWindlessCharacter::PressQuickSlotConsume2()
{
	if (quickSlots)
		quickSlots->HandleSlotPressed(EQuickSlot::EQS_Consume2);
}

void AWindlessCharacter::PressQuickSlotConsume3()
{
	if (quickSlots)
		quickSlots->HandleSlotPressed(EQuickSlot::EQS_Consume3);
}

void AWindlessCharacter::PressQuickSlotConsume4()
{
	if (quickSlots)
		quickSlots->HandleSlotPressed(EQuickSlot::EQS_Consume4);
}

void AWindlessCharacter::ReleaseQuickSlotRecovery()
{
	if (quickSlots)
		quickSlots->HandleSlotReleased(EQuickSlot::EQS_Recovery);
}

void AWindlessCharacter::ReleaseQuickSlotConsume1()
{
	if (quickSlots)
		quickSlots->HandleSlotReleased(EQuickSlot::EQS_Consume1);
}

void AWindlessCharacter::ReleaseQuickSlotConsume2()
{
	if (quickSlots)
		quickSlots->HandleSlotReleased(EQuickSlot::EQS_Consume2);
}

void AWindlessCharacter::ReleaseQuickSlotConsume3()
{
	if (quickSlots)
		quickSlots->HandleSlotReleased(EQuickSlot::EQS_Consume3);
}

void AWindlessCharacter::ReleaseQuickSlotConsume4()
{
	if (quickSlots)
		quickSlots->HandleSlotReleased(EQuickSlot::EQS_Consume4);
}

void AWindlessCharacter::QuickSlotCancel()
{
	if (quickSlots)
		quickSlots->HandleCancelInput();
}

void AWindlessCharacter::HandleQuickSlotChanged(EQuickSlot quickSlot)
{
	RefreshQuickSlotHUD();
}

void AWindlessCharacter::InitializeWindlessOverlay()
{
	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (playerController)
	{
		AWindlessHUD* windlessHUD = Cast<AWindlessHUD>(playerController->GetHUD());
		if (windlessHUD)
		{
			windlessOverlay = windlessHUD->GetWindlessOverlay();
			if (windlessOverlay && attributes)
			{
				if (bodyParts)
					bodyParts->BroadcastAllParts();
				windlessOverlay->SetStaminaBarPercent(1.f);
				windlessOverlay->SetChaosBarPercent(attributes->GetChaosPercent());
				windlessOverlay->SetMoltStones(attributes->GetMoltStones());
			}
		}
	}
	RefreshQuickSlotHUD();
}

void AWindlessCharacter::SetHUDStamina()
{
	if (windlessOverlay && attributes)
		windlessOverlay->SetStaminaBarPercent(attributes->GetStaminaPercent());
}

void AWindlessCharacter::SetHUDChaos()
{
    if (windlessOverlay && attributes)
    {
        windlessOverlay->SetChaosBarPercent(attributes->GetChaosPercent());
        windlessOverlay->SetStaminaLockRatio(attributes->GetStaminaLockRatio());
    }
}

void AWindlessCharacter::BindCallbacks()
{
	/* 부위를 변경하고, UI와 사망의 콜백을 할당하는 코드 */
	if (bodyParts)
		bodyParts->onBodyPartChanged.AddDynamic(this, &AWindlessCharacter::HandleBodyPartChanged);

	/* 탈 것 콜백을 할당하는 코드 */
	if (mountComponent)
		mountComponent->OnMountPhaseChanged.AddDynamic(this, &AWindlessCharacter::HandleMountPhaseChanged);

	/* 속성 관련 콜백을 할당하는 코드 */
	if (attributes)
	{
		attributes->onChaosStateChanged.AddDynamic(this, &AWindlessCharacter::HandleChaosStateChanged);
		attributes->onMoltingChanged.AddDynamic(this, &AWindlessCharacter::HandleMoltingChanged);
	}

	/* 인벤토리 관련 콜백을 할당하는 코드 */
	if (inventory)
	{
		inventory->OnItemDropped.AddDynamic(this, &AWindlessCharacter::HandleItemDropped);
		inventory->OnQuickSlotChanged.RemoveDynamic(this, &AWindlessCharacter::HandleQuickSlotChanged);
		inventory->OnQuickSlotChanged.AddDynamic(this, &AWindlessCharacter::HandleQuickSlotChanged);
		inventory->canChangeEquipSlot.BindUObject(this, &AWindlessCharacter::CanChangeEquipSlot);
	}

	/* 퀵슬롯 관련 콜백을 할당하는 코드 */
	if (quickSlots)
	{
		quickSlots->OnHighlightChanged.RemoveDynamic(this, &AWindlessCharacter::HandleQuickSlotHighlightChanged);
		quickSlots->OnHighlightChanged.AddDynamic(this, &AWindlessCharacter::HandleQuickSlotHighlightChanged);
	}

	/* 정비 모드 관련 콜백을 할당하는 코드 */
	if (maintenance)
		maintenance->onMaintenanceStateChanged.AddDynamic(this, &AWindlessCharacter::HandleMaintenanceStateChanged);

	/* 강한 경직 콜백을 할당하는 코드 */
	if (staggerComponent)
		staggerComponent->onStrongStaggerEnded.BindUObject(this, &AWindlessCharacter::HandleStrongStaggerEnded);
}

void AWindlessCharacter::InitializePostProcessingEffect()
{
	if (vignetteMaterial && viewCamera)
	{
		vignetteMID = UMaterialInstanceDynamic::Create(vignetteMaterial, this);
		if (vignetteMID)
		{
			vignetteMID->SetScalarParameterValue(TEXT("VignetteStrength"), 0.f);
			viewCamera->PostProcessSettings.AddBlendable(vignetteMID, 1.0f);
		}
	}
}
