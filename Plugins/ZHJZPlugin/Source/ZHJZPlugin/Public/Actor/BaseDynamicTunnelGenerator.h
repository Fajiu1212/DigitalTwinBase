#pragma once

#include "CoreMinimal.h"
#include "AwsaWebUI_Subsystem.h"
#include "Components/DynamicMeshComponent.h"
#include "Func/ZHJZ_Interface.h"
#include "GameFramework/Actor.h"
#include "BaseDynamicTunnelGenerator.generated.h"

class USplineComponent;

USTRUCT(Blueprintable)
struct FMileWidgetActorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float SplinePercent = 0.f;
	UPROPERTY(BlueprintReadWrite)
	FString Text = TEXT("");

	FMileWidgetActorInfo() = default;

	FMileWidgetActorInfo(float InSplinePercent, const FString& InText)
		: SplinePercent(InSplinePercent),
		  Text(InText)
	{
	}
};

USTRUCT(Blueprintable)
struct FStakeWidgetActorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float SplinePercent = 0.f;
	UPROPERTY(BlueprintReadWrite)
	FString Text = TEXT("");

	FStakeWidgetActorInfo() = default;

	FStakeWidgetActorInfo(float InSplinePercent, const FString& InText)
		: SplinePercent(InSplinePercent),
		  Text(InText)
	{
	}
};

UCLASS()
class ZHJZPLUGIN_API ABaseDynamicTunnelGenerator : public AActor, public IZHJZ_Interface
{
	GENERATED_BODY()

	ABaseDynamicTunnelGenerator();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void MouseClick_Implementation(bool isBool) override;

public:
	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	FString ModuleName = TEXT("进度管理");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	bool bIsMultiple = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	bool bIsSectionMode = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	bool bUseComplexCollision = true;
	UPROPERTY(EditAnywhere, Category = "Procedural")
	UStaticMesh* SourceStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Dynamic Tunnel Generator")
	UMaterialInterface* Material_Completed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Dynamic Tunnel Generator")
	UMaterialInterface* Material_UnFinished;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	bool bFlip;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	TArray<float> PercentArr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Dynamic Tunnel Generator")
	bool bUseCustomSpline = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	USplineComponent* CustomSplineComponent;

	// Tunnel Params
	// 所属隧洞
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Dynamic Tunnel Generator")
	FString OwnerTunnelName = TEXT("");
	// 所属层级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	int32 OwnerLayer = 1;
	// 所属左右幅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	ETunnelSide OwnerSide;
	// 里程方向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	TArray<ETunnelMileageDirection> OwnerMileageDirections;
	// 起始工作面名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	FString BeginWorkFaceName = TEXT("");
	// 结束工作面名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	FString EndWorkFaceName = TEXT("");
	// 工作面里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	float Mileage = 0.0f;
	// 今日里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	float TodayMileage = 0.0f;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Dynamic Tunnel Generator")
	void RebuildDynamicMesh();

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dynamic Tunnel Generator")
	// TArray<UDynamicMeshComponent*> DynamicMeshComponents;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Dyanmic Tunnel Generator")
	UDynamicMeshComponent* DynamicMeshComponent_0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Dyanmic Tunnel Generator")
	UDynamicMeshComponent* DynamicMeshComponent_1;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Dyanmic Tunnel Generator")
	UDynamicMeshComponent* DynamicMeshComponent_2;

	UPROPERTY()
	UDynamicMesh* OriginalDynamicMesh_0;
	UPROPERTY()
	UDynamicMesh* OriginalDynamicMesh_1;
	UPROPERTY()
	UDynamicMesh* OriginalDynamicMesh_2;

	// UPROPERTY()
	// TMap<int32, UDynamicMesh*> DynamicMeshes;
protected:
	bool bIsUpdatingComponents = false;

	// UFUNCTION(CallInEditor, BlueprintCallable, Category = "Dynamic Tunnel Generator")
	// void EnsureDynamicMeshComponents();
	UDynamicMeshComponent* CreateDynamicMeshComponent(int32 Index);
	void DestroyDynamicMeshComponent(UDynamicMeshComponent* Comp);

	UDynamicMesh* CopyMeshFromSourceStaticMesh(UDynamicMeshComponent* TargetComponent,
	                                           bool bResetTargetMesh);

	UPROPERTY(BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	USplineComponent* SplineComponent;
	void SetDMMaterials();
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Dynamic Tunnel Generator")
	void BuildSpline();
	void CutDynamicMesh();

	// Web request delegate
	UFUNCTION()
	void OnProgressLoadSuccess(const FString& TargetModuleName);
	UFUNCTION()
	void OnModuleSwitched(const FString& TargetModule, const FString& InExtraParam);

	// Explosion animation
	UFUNCTION()
	void OnExplosionAnimation(const FString& OwnerName, bool bMultiple);
	bool bExplosion = false;

	FTimerHandle MoveTimerHandle;
	float MoveDistance = 6000.0f; // 总移动距离
	float MoveDuration = 2.0f; // 总时间
	FVector BasicLoc = FVector::ZeroVector;

	float TargetZOffset;
	float ElapsedTime;
	bool bIsMovingUp;
	UFUNCTION(BlueprintCallable, Category = "Dynamic Tunnel Generator")
	void MoveActorZSmoothly(bool bMoveUp = true);
	UFUNCTION()
	void UpdateMove();

	// Section Cut
	UFUNCTION()
	void OnSectionCuttingSelected(const FString& TargetTunnelName);

	UFUNCTION(BlueprintImplementableEvent, Category="Dynamic Tunnel Generator")
	void CreateVehicle();
	UFUNCTION(BlueprintImplementableEvent, Category="Dynamic Tunnel Generator")
	void DestroyVehicle();

	// // Vehicle Request
	// UFUNCTION()
	// void OnCarRequestSuccess();

	// Mile widget actors
	UPROPERTY(BlueprintReadWrite, Category = "Dynamic Tunnel Generator")
	TArray<FMileWidgetActorInfo> MileWidgetActorInfos;
	UPROPERTY(blueprintreadonly, Category = "Dynamic Tunnel Generator")
	TArray<FStakeWidgetActorInfo> StakeWidgetActorInfos;

public:
	UFUNCTION(BlueprintImplementableEvent, Category="Dynamic Tunnel Generator")
	void ShowMileWidgetActors(bool bShow);
	UFUNCTION(BlueprintImplementableEvent, Category="Dynamic Tunnel Generator")
	void ShowStakeWidgetActors(bool bShow);
};
