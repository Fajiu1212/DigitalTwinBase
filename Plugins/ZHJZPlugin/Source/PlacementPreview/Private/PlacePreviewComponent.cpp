#include "PlacePreviewComponent.h"

#include "Kismet/GameplayStatics.h"
#include "PlacePreviewMarker.h"
#include "PlacePreviewPoi.h"
#include "Blueprint/AsyncTaskDownloadImage.h"


class AAwsaPlayerPawn;

UPlacePreviewComponent::UPlacePreviewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UPlacePreviewComponent::BeginPlay()
{
	Super::BeginPlay();

	SetDetectionActive(false);

	if (GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		PreviewMarker = GetWorld()->SpawnActor<APlacePreviewMarker>(APlacePreviewMarker::StaticClass(),
		                                                            FVector::ZeroVector, FRotator::ZeroRotator,
		                                                            SpawnParams);
		
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		                                       [this]()
		                                       {
		                                       	if (PreviewMarker)
		                                       	{
													   PreviewMarker->SetActorHiddenInGame(true);
													   OnRaycastHit.AddDynamic(PreviewMarker, &APlacePreviewMarker::OnRaycastHit);
													   if (PreviewMarker->GetPlacePreviewPoiWidget())
													   {
														   OnRaycastHit.AddDynamic(PreviewMarker->GetPlacePreviewPoiWidget(), &UPlacePreviewPoi::OnRaycastHit);
													   }
												   }
		                                       }, 0.5f, false);
		
	}
}

void UPlacePreviewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetDetectionActive(false);
	Super::EndPlay(EndPlayReason);
}


void UPlacePreviewComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlacePreviewComponent::SetDetectionActive(bool bActive)
{
	if (bIsRaycastActive == bActive)
	{
		return;
	}

	bIsRaycastActive = bActive;
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bIsRaycastActive)
	{
		World->GetTimerManager().SetTimer(
			RaycastTimerHandle,
			this,
			&UPlacePreviewComponent::PerformRaycast,
			0.1f,
			true
		);
	}
	else
	{
		World->GetTimerManager().ClearTimer(RaycastTimerHandle);
	}
}

void UPlacePreviewComponent::OnLeftMouseClicked()
{
	Raycast();
	PreviewMarker->ShowPoiWidget(true);
	PreviewMarker->GetPlacePreviewPoiWidget()->ClipBoardCopy();
}

void UPlacePreviewComponent::ShowMarker(bool bShow)
{
	if (PreviewMarker)
	{
		PreviewMarker->SetActorHiddenInGame(!bShow);
	}
}

void UPlacePreviewComponent::ShowWidget(bool bShow)
{
	if (PreviewMarker)
	{
		PreviewMarker->ShowPoiWidget(bShow);
	}
}

void UPlacePreviewComponent::PerformRaycast()
{
	if (!bIsRaycastActive)
	{
		return;
	}
	Raycast();
}

void UPlacePreviewComponent::Raycast()
{
	FVector RayStart, RayDirection;
	if (!GetScreenCenterRay(RayStart, RayDirection))
	{
		return;
	}

	const FVector RayEnd = RayStart + (RayDirection * MaxDistance);

	FHitResult HitResult;
	ECollisionChannel CollisionChannel = ECC_Visibility;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		RayStart,
		RayEnd,
		CollisionChannel,
		CollisionParams
	);

	if (bDebugAlways && bHit)
	{
		const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(GetWorld(), RayStart, RayEnd, DebugColor, false, 100.f, 0, 2.0f);
	}

	if (bHit && OnRaycastHit.IsBound())
	{
		OnRaycastHit.Broadcast(
			HitResult.Location,
			HitResult.GetActor(),
			HitResult.GetComponent()
		);
	}
}

bool UPlacePreviewComponent::GetScreenCenterRay(FVector& OutScreenLoc, FVector& OutScreenDir)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!PC)
	{
		return false;
	}

	float ViewportSizeX = 0, ViewportSizeY = 0;
	// PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	// const FVector2D ScreenCenter = FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
	PC->GetMousePosition(ViewportSizeX, ViewportSizeY);

	return PC->DeprojectScreenPositionToWorld(
		ViewportSizeX,
		ViewportSizeY,
		OutScreenLoc,
		OutScreenDir
	);

}
