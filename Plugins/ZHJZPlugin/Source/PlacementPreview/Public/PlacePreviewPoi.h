#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlacePreviewPoi.generated.h"

class UTextBlock;

UCLASS()
class PLACEMENTPREVIEW_API UPlacePreviewPoi : public UUserWidget
{
	GENERATED_BODY()
	
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void BeginDestroy() override;

public:
	UPROPERTY(VisibleAnywhere,meta=(BindWidget))
	UTextBlock* LocationTextBlock_X;
	UPROPERTY(VisibleAnywhere,meta=(BindWidget))
	UTextBlock* LocationTextBlock_Y;
	UPROPERTY(VisibleAnywhere,meta=(BindWidget))
	UTextBlock* LocationTextBlock_Z;

	UFUNCTION()
	void OnRaycastHit(FVector HitLocation, AActor* HitActor, UPrimitiveComponent* HitComponent);
	UFUNCTION()
	void UpdateTargetLocation(FVector TargetLocation);
	UFUNCTION()
	void ClipBoardCopy();
};
