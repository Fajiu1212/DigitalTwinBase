#pragma once

#include "CoreMinimal.h"
#include "AwsaWebUI_Subsystem.h"
#include "Func/ZHJZ_Interface.h"
#include "GameFramework/Actor.h"
#include "BaseGeologicalForecastGenerator.generated.h"

class UDynamicMeshComponent;
class UDynamicMesh;
class USplineComponent;

USTRUCT(Blueprintable)
struct FCutCenterInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float CutPercent = 0.f;
	UPROPERTY(BlueprintReadWrite)
	int32 LevelIndex = 0;

	FCutCenterInfo() = default;
	FCutCenterInfo(float InCutPercent, int32 InLevelIndex)
		: CutPercent(InCutPercent),
		  LevelIndex(InLevelIndex)
	{
	}
};
UCLASS()
class ZHJZPLUGIN_API ABaseGeologicalForecastGenerator : public AActor , public IZHJZ_Interface
{
	GENERATED_BODY()

	ABaseGeologicalForecastGenerator();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// ZHJZ_Interface implementation
	virtual void MouseOverlay_Component_Implementation(UMeshComponent* TargetComponent) override;
	virtual void MouseLeave_Component_Implementation(UMeshComponent* TargetComponent) override;
	virtual void MouseClick_Component_Implementation(UMeshComponent* TargetComponent) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	UStaticMeshComponent* BaseStaticMeshComponent;
	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	FString TunnelName = TEXT("");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	float TotalTunnelLength = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	UStaticMesh* BaseStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	UStaticMesh* SourceStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	TArray<UMaterialInterface*> MaterialInterfaces_Levels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	UMaterialInterface* MaterialInterfaces_Source;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	bool bFlip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Geological Forecast Generator")
	bool bUseCustomSpline = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geological Forecast Generator")
	USplineComponent* CustomSplineComponent;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Geological Forecast Generator")
	void RebuildSpline();
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Geological Forecast Generator")
	void AddDM();
protected:
	bool bIsUpdatingComponents = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	UDynamicMeshComponent* SampleDynamicMeshComponent;
	// Copy static mesh to DynamicMesh
	UDynamicMesh* CopyMeshFromSourceStaticMesh(UDynamicMeshComponent* TargetComponent,
											   bool bResetTargetMesh);
	UPROPERTY(BlueprintReadWrite, Category = "Geological Forecast Generator")
	USplineComponent* SplineComponent;
	UPROPERTY()
	TArray<UDynamicMeshComponent*> CutDynamicMeshComponents;
	UPROPERTY(BlueprintReadWrite, Category = "Geological Forecast Generator")
	TArray<FCutCenterInfo> CutMiddlePercents;
	UPROPERTY()
	FGeologicalForecastInfo GeologicalForecastInfo;
	void CutDynamicMesh();
	// Web request delegate
	UFUNCTION()
	void OnGeologicalForecastLoadSuccess();

	UDynamicMeshComponent* CreateDynamicMeshComponent();
	void AddCutDynamicMeshComponents(const float BeginPercent, const float EndPercent, int32 MtlLevel, int32 ID);
	void SetDynamicMeshMaterial(UDynamicMeshComponent* TargetDynamicMesh, int32 MaterialIndex);

	// Try get component's tag
	static bool TryExtractGeologicalForecastId(const UActorComponent* Component, int32& OutId);

	// Module switch
	UFUNCTION()
	void OnModuleSwitched(const FString& InModuleName, const FString& InExtraParam);

	// 超前地质预报
	bool bIsGeoWidgetVisible = false;
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Geological Forecast Generator")
	void ShowCenterWidget(bool bShow);
	
	// 超限挖
	UPROPERTY(BlueprintReadWrite, Category = "Geological Forecast Generator")
	TArray<FSingleOverExcavationInfo> SlidesArr;
	UFUNCTION()
	void OnOverExcavationLoadSuccess();
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable,Category = "Geological Forecast Generator")
	void ShowSlides(bool bShow);
	bool bIsSlidesShown = false;

	// 围岩变形
	UPROPERTY(BlueprintReadWrite, Category = "Geological Forecast Generator")
	TArray<FSingleSurface> SurfacesArr;
	UFUNCTION()
	void OnSurfaceLoadSuccess();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Geological Forecast Generator")
	void ShowSurfaces(bool bShow);
	bool bIsSurfaceShown = false;
};
