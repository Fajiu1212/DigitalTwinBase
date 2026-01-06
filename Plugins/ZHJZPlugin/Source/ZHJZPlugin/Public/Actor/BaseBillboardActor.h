#pragma once

#include "CoreMinimal.h"
#include "Func/ZHJZ_Interface.h"
#include "GameFramework/Actor.h"
#include "BaseBillboardActor.generated.h"

class AAwsaPlayerPawn;
class UWidgetComponent;
class AStaticMeshActor;

UCLASS()
class ZHJZPLUGIN_API ABaseBillboardActor : public AActor , public IZHJZ_Interface
{
	GENERATED_BODY()

public:
	ABaseBillboardActor();
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void MouseClick_Implementation(bool isBool) override;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Billboard", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MeshComponentsRoot = nullptr;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Billboard", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* BillboardWidgetComp = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard",
		meta = (DisplayPriority = 0, DisplayName = "Billboard Static Meshes", AllowPrivateAccess = "true"))
	TArray<UStaticMesh*> BillboardStaticMeshes;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard",
		meta = (DisplayPriority = 0, DisplayName = "ComponentString"))
	FString ComponentStr = TEXT("");
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard",
		meta = (DisplayPriority = 0, DisplayName = "Icon"))
	UTexture2D* BillboardIcon = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard",
		meta = (DisplayPriority = 0, DisplayName = "BindAura"))
	AStaticMeshActor* BillboardBindAura = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Billboard")
	TSubclassOf<UUserWidget> BillboardWidget = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard")
	FVector2D WidgetDrawSize = FVector2D(174,56);
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard")
	float RefArmLength = 2000.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard")
	float ClampMaxScale = 20.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Billboard")
	float ClampMinScale = 1.f;
	
private:
	UPROPERTY()
	AAwsaPlayerPawn* PlayerPawn = nullptr;
	FTimerHandle RotateTimerHandle;
	float LastArmLength = 0.f;
	float AdjustedScale = 0.f;
	
	AAwsaPlayerPawn* GetMainPawn();
	UFUNCTION()
	void UpdateMeshComponents();
	void SetWidget();
	void FaceCameraAndFitScale();

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Billboard")
	void OnMouseClicked(bool bIsBool);
	UFUNCTION(BlueprintCallable, Category = "Billboard")
	void ToggleBillboardVisibility(bool bShow);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Billboard")
	void SetBillboardIcon();
};
