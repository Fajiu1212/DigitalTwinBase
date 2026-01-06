#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaterialBasedTunnelProgress.generated.h"

class USplineComponent;

UCLASS()
class ZHJZPLUGIN_API AMaterialBasedTunnelProgress : public AActor
{
	GENERATED_BODY()

	AMaterialBasedTunnelProgress();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// virtual void MouseClick_Implementation(bool isBool) override;

public:
	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Based Tunnel Progress")
	FString TunnelName = TEXT("");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	FString ModuleName = TEXT("进度管理");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Based Tunnel Progress")
	int32 SLabIndex = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Based Tunnel Progress")
	bool bFlip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Based Tunnel Progress")
	UStaticMesh* SourceStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Material Based Tunnel Progress")
	UMaterialInterface* Material_Progress;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Material Based Tunnel Progress")
	FTransform SourceTransform = FTransform::Identity;

protected:
	UPROPERTY()
	TArray<UStaticMeshComponent*> CreatedStaticeMeshComponents;
	// Web request delegate
	UFUNCTION()
	void OnLoadSuccess(int32 InSlabIndex, const FString& TargetModuleName);
	UFUNCTION()
	void OnModuleSwitched(const FString& NewModuleName, const FString& InExtraParam);

	// Explosion animation
	UFUNCTION()
	void OnExplosionAnimation(const FString& OwnerName, bool bIsMultiple);
	bool bExplosion = false;
	
	FTimerHandle MoveTimerHandle;
	float MoveDistance = 6000.0f; // 总移动距离
	float MoveDuration = 2.0f;    // 总时间
	FVector BasicLoc = FVector::ZeroVector;
	
	float TargetZOffset;
	float ElapsedTime;
	bool bIsMovingUp;
	UFUNCTION(BlueprintCallable, Category = "Material Based Tunnel Progress")
	void MoveActorZSmoothly(bool bMoveUp = true);
	UFUNCTION()
	void UpdateMove();

	UFUNCTION()
	void OnSectionCuttingSelected(const FString& TargetTunnelName);
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	bool bNeedPoi = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	USplineComponent* PoiSplineComponent = nullptr;
};
