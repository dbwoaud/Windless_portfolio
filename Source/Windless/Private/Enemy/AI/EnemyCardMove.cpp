#include "Enemy/AI/EnemyCardMove.h"

FVector EnemyCardMove::GetCirclePoint(const FVector& center, const FVector& from, float radius, float stepDistance, float direction)
{
	FVector offset = from - center;
	offset.Z = 0.f;
	if (offset.IsNearlyZero() || radius <= KINDA_SMALL_NUMBER)
		return from;

	const float angleDegrees = FMath::RadiansToDegrees(stepDistance / radius) * FMath::Sign(direction);
	const FVector rotated = offset.GetSafeNormal().RotateAngleAxis(angleDegrees, FVector::UpVector) * radius;
	return FVector(center.X + rotated.X, center.Y + rotated.Y, from.Z);
}

FVector EnemyCardMove::GetBackOffPoint(const FVector& from, const FVector& target, float distance)
{
	FVector away = from - target;
	away.Z = 0.f;
	if (!away.Normalize())
		away = FVector::ForwardVector;

	return from + away * distance;
}

float EnemyCardMove::GetApproachSpeed(float distance, float runDistance, float walkSpeed, float runSpeed)
{
	return distance > runDistance ? runSpeed : walkSpeed;
}