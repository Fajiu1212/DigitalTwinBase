#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaticBasedTunnelProgress.generated.h"

UCLASS()
class ZHJZPLUGIN_API AStaticBasedTunnelProgress : public AActor
{
	GENERATED_BODY()
	
	AStaticBasedTunnelProgress();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Based Tunnel Progress")
	FString TunnelName = TEXT("");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Based Tunnel Progress")
	FString ModuleName = TEXT("进度管理");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Based Tunnel Progress")
	UStaticMesh* SourceStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Static Based Tunnel Progress")
	UMaterialInstance* SourceMaterial = nullptr;

protected:
	UPROPERTY()
	UStaticMeshComponent* StaticTunnelMeshComponent;
	
	// Web request delegate
	UFUNCTION()
	void OnLoadSuccess(const FString& TargetModuleName);
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
};
