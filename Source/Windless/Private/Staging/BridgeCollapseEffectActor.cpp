#include "Staging/BridgeCollapseEffectActor.h"
#include "Windless/DebugMacro.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

void ABridgeCollapseEffectActor::BeginPlay()
{
	Super::BeginPlay();
	SetConstraints();
}

void ABridgeCollapseEffectActor::HandlePlay()
{
	if (breakStartIndex < 0 || breakEndIndex < 0)
		return;

	const int32 startIdx = FMath::Min(breakStartIndex, breakEndIndex);
	const int32 endIdx = FMath::Max(breakStartIndex, breakEndIndex);

	auto BreakAt = [this](int32 i)
		{
			if (constraints.IsValidIndex(i) && constraints[i])
			{
				constraints[i]->BreakConstraint();
				constraints[i]->DestroyComponent();
				constraints[i] = nullptr;
				UE_LOG(LogWindless, Log, TEXT("컨스트레인트 파괴: 인덱스 %d"), i);
			}
		};

	BreakAt(startIdx - 1);
	BreakAt(endIdx);

	for (int32 s = startIdx; s <= endIdx; ++s)
	{
		if (collapsingSegments.IsValidIndex(s))
		{
			if (UStaticMeshComponent* m = GetSegmentMesh(collapsingSegments[s]))
			{
				m->SetLinearDamping(0.1f); 
				m->SetAngularDamping(0.5f);
				m->WakeRigidBody();
			}
		}
	}

	UE_LOG(LogWindless, Warning, TEXT("다리 붕괴 구간 [%d~%d]: %s"), startIdx, endIdx, *GetName());
}

void ABridgeCollapseEffectActor::SetConstraints()
{
	for (AActor* seg : collapsingSegments)
	{
		if (UStaticMeshComponent* m = GetSegmentMesh(seg))
		{
			m->SetMobility(EComponentMobility::Movable);
			m->SetSimulatePhysics(true);
			m->SetLinearDamping(1.0f);
			m->SetAngularDamping(5.0f);
		}
	}

	for (int32 i = 0; i < collapsingSegments.Num() - 1; ++i)
	{
		UStaticMeshComponent* a = GetSegmentMesh(collapsingSegments[i]);
		UStaticMeshComponent* b = GetSegmentMesh(collapsingSegments[i + 1]);
		if (!a || !b)
			continue;

		UPhysicsConstraintComponent* c = NewObject<UPhysicsConstraintComponent>(this);
		c->RegisterComponent();
		c->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		c->SetWorldLocation((a->GetComponentLocation() + b->GetComponentLocation()) * 0.5f);

		c->SetLinearXLimit(LCM_Locked, 0.f);
		c->SetLinearYLimit(LCM_Locked, 0.f);
		c->SetLinearZLimit(LCM_Limited, 30.f);

		c->SetAngularSwing1Limit(ACM_Limited, 15.f);
		c->SetAngularSwing2Limit(ACM_Locked, 0.f);
		c->SetAngularTwistLimit(ACM_Locked, 0.f);

		c->SetLinearBreakable(false, 0.0f);
		c->SetAngularBreakable(false, 0.0f);
		c->SetConstrainedComponents(a, NAME_None, b, NAME_None);
		c->SetDisableCollision(true);
		constraints.Add(c);
	}

	UPrimitiveComponent* rootPrim = Cast<UPrimitiveComponent>(RootComponent);
	auto AnchorToWorld = [this, rootPrim](AActor* seg)
		{
			UStaticMeshComponent* m = GetSegmentMesh(seg);
			if (!m)
				return;

			UPhysicsConstraintComponent* anchor = NewObject<UPhysicsConstraintComponent>(this);
			anchor->RegisterComponent();
			anchor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			anchor->SetWorldLocation(m->GetComponentLocation());
			anchor->SetLinearXLimit(LCM_Locked, 0.f);
			anchor->SetLinearYLimit(LCM_Locked, 0.f);
			anchor->SetLinearZLimit(LCM_Locked, 0.f);
			anchor->SetAngularSwing1Limit(ACM_Locked, 0.f);
			anchor->SetAngularSwing2Limit(ACM_Locked, 0.f);
			anchor->SetAngularTwistLimit(ACM_Locked, 0.f);
			anchor->SetConstrainedComponents(rootPrim, NAME_None, m, NAME_None);
		};

	if (bAnchorStart && collapsingSegments.Num() > 0)
		AnchorToWorld(collapsingSegments[0]);
	if (bAnchorEnd && collapsingSegments.Num() > 1)
		AnchorToWorld(collapsingSegments.Last());

	UE_LOG(LogWindless, Log, TEXT("다리 컨스트레인트 %d개 생성: %s"), constraints.Num(), *GetName());
}

UStaticMeshComponent* ABridgeCollapseEffectActor::GetSegmentMesh(AActor* seg) const
{
	return seg ? Cast<UStaticMeshComponent>(seg->GetRootComponent()) : nullptr;
}