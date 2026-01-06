#include "Gameplay/AwsaPlayerController.h"

AAwsaPlayerController::AAwsaPlayerController(const class FObjectInitializer& ObjectInitializer)
{
}

bool AAwsaPlayerController::GetMousePositionInWorld(FVector& OutWorldPosition)
{
	float MouseX, MouseY;
	if (!GetMousePosition(MouseX, MouseY))
	{
		return false;
	}

	FVector WorldLocation, WorldDirection;
	if (!DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
	{
		return false;
	}

	FVector Start = WorldLocation;
	FVector End = Start + (WorldDirection * 1000000.0f);

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.bTraceComplex = true;
	CollisionParams.AddIgnoredActor(GetPawn()); 

	if (GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_WorldStatic,
		CollisionParams))
	{
		OutWorldPosition = HitResult.Location;

		DrawDebugSphere(GetWorld(), HitResult.Location, 20.0f, 12, FColor::Green, false, 2.0f);
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f);

		return true;
	}

	OutWorldPosition = End;
	return false;
}
