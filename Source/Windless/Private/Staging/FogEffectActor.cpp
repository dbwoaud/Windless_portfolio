#include "Staging/FogEffectActor.h"
#include "Windless/DebugMacro.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"

AFogEffectActor::AFogEffectActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AFogEffectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bBlending || !fogComponent)
		return;

	blendAlpha = FMath::Clamp(blendAlpha + DeltaTime / blendTime, 0.f, 1.f);

	fogComponent->SetFogDensity(FMath::Lerp(currentDensity, targetDensity, blendAlpha));
	fogComponent->SetFogInscatteringColor(FMath::Lerp(currentColor, targetInscatteringColor, blendAlpha));
	fogComponent->SetFogHeightFalloff(FMath::Lerp(currentFalloff, targetHeightFalloff, blendAlpha));

	if (blendAlpha >= 1.f) // 목표 설정 값 도달 시
	{
		bBlending = false;
		SetActorTickEnabled(false);
		UE_LOG(LogWindless, Log, TEXT("안개 블렌딩 완료: %s"), *GetName());
	}
}

void AFogEffectActor::HandlePlay()
{
	if (!targetFog)
	{
		UE_LOG(LogWindless, Warning, TEXT("대상 안개 액터가 지정되지 않음: %s"), *GetName());
		return;
	}

	fogComponent = targetFog->FindComponentByClass<UExponentialHeightFogComponent>();
	if (!fogComponent)
		return;

	currentDensity = fogComponent->FogDensity;
	currentColor = fogComponent->FogInscatteringLuminance;
	currentFalloff = fogComponent->FogHeightFalloff;

	blendAlpha = 0.f;
	bBlending = true;
	SetActorTickEnabled(true);
}