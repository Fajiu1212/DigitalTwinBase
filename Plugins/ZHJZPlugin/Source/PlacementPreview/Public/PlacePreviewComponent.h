#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/TypeHash.h"
#include "PlacePreviewComponent.generated.h"

class UImage;
struct FJsonStruct;
class APlacePreviewMarker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FPrivewRaycastHitEvent,
	FVector, HitLocation,
	AActor*, HitActor,
	UPrimitiveComponent*, HitComponent
);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnImageDownloaded, UTexture2D*, Texture);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLACEMENTPREVIEW_API UPlacePreviewComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlacePreviewComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Preview static mesh component
	UPROPERTY()
	APlacePreviewMarker* PreviewMarker;
	// Place preview raycast
public:
	// Result
	UPROPERTY(BlueprintAssignable, Category = "PlacementPreview|Raycast")
	FPrivewRaycastHitEvent OnRaycastHit;
	// Toggle raycast active
	UFUNCTION(BlueprintCallable, Category = "PlacementPreview|Raycast")
	void SetDetectionActive(bool bActive);

	// On left mouse clicked
	UFUNCTION()
	void OnLeftMouseClicked();
	// Show marker
	void ShowMarker(bool bShow);
	// Show widget
	void ShowWidget(bool bShow);
protected:
	// Specific raycast logic
	void PerformRaycast();
	void Raycast();
	bool GetScreenCenterRay(FVector& OutScreenLoc, FVector& OutScreenDir);
	
	// Runtime param
protected:
	UPROPERTY(Transient,BlueprintReadOnly,Category="PlacementPreview|Raycast")
	bool bIsRaycastActive = false;
	UPROPERTY(Transient, BlueprintReadOnly, Category="PlacementPreview|Raycast")
	FTimerHandle RaycastTimerHandle;
	UPROPERTY(Transient, BlueprintReadOnly, Category="PlacementPreview|Raycast")
	float MaxDistance = 10000000.f;
	UPROPERTY(Transient, EditAnywhere, Category="PlacementPreview|Raycast")
	bool bDebugAlways = false;
};
