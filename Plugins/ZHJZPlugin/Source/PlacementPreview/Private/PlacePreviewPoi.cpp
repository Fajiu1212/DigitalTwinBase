#include "PlacePreviewPoi.h"

#include "Windows/WindowsPlatformApplicationMisc.h"
#include "Components/TextBlock.h"

bool UPlacePreviewPoi::Initialize()
{
	return Super::Initialize();
}

void UPlacePreviewPoi::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlacePreviewPoi::BeginDestroy()
{
	Super::BeginDestroy();
}

void UPlacePreviewPoi::OnRaycastHit(FVector HitLocation, AActor* HitActor, UPrimitiveComponent* HitComponent)
{
	UpdateTargetLocation(HitLocation);
}

void UPlacePreviewPoi::UpdateTargetLocation(FVector TargetLocation)
{
	FString XStr = FString::Printf(TEXT("%.2f"), TargetLocation.X).Replace(TEXT(","), TEXT(""));
	FString YStr = FString::Printf(TEXT("%.2f"), TargetLocation.Y).Replace(TEXT(","), TEXT(""));
	FString ZStr = FString::Printf(TEXT("%.2f"), TargetLocation.Z).Replace(TEXT(","), TEXT(""));

	if (LocationTextBlock_X && LocationTextBlock_Y && LocationTextBlock_Z)
	{
		LocationTextBlock_X->SetText(FText::FromString(XStr));
		LocationTextBlock_Y->SetText(FText::FromString(YStr));
		LocationTextBlock_Z->SetText(FText::FromString(ZStr));
	}
}

void UPlacePreviewPoi::ClipBoardCopy()
{
	FString XStr = LocationTextBlock_X->GetText().ToString();
	FString YStr = LocationTextBlock_Y->GetText().ToString();
	FString ZStr = LocationTextBlock_Z->GetText().ToString();

	FString TargetLocation = FString::Printf(TEXT("%s，%s，%s"),
	                                         *XStr,
	                                         *YStr,
	                                         *ZStr);

	FPlatformApplicationMisc::ClipboardCopy(*TargetLocation);
}
