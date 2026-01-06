#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlacePreviewMarker.generated.h"

class UPlacePreviewPoi;
class UWidgetComponent;

UCLASS()
class PLACEMENTPREVIEW_API APlacePreviewMarker : public AActor
{
	GENERATED_BODY()

public:
	APlacePreviewMarker();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootComp;
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* WidgetComp;
	UPROPERTY()
	UPlacePreviewPoi* PlacePreviewPoiWidget;
	
	FVector BaseLocation = FVector::ZeroVector;
	float RunningTime = 0.0f;
	bool bIsBouncing = false;
public:
	UPROPERTY(EditAnywhere)
	UMaterial* MarkerMaterialInstance;
	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstanceDynamic;
	void InitMarkerMesh(UStaticMesh* TargetMesh);
	void SetBaseLocation(const FVector& NewLocation);
	void StartBouncing();
	void StopBouncing();
	void ShowPoiWidget(bool bShow);
	bool IsBouncing() const { return bIsBouncing; }
	UPlacePreviewPoi* GetPlacePreviewPoiWidget() const { return PlacePreviewPoiWidget; }
	float Amplitude = 50.0f;
	float Frequency = 1.0f;

	UFUNCTION()
	void OnRaycastHit(FVector HitLocation, AActor* HitActor, UPrimitiveComponent* HitComponent);
};
