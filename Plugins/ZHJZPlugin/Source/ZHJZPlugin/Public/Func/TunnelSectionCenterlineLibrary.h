#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "DynamicMeshActor.h"
#include "TunnelSectionCenterlineLibrary.generated.h"

UCLASS()
class ZHJZPLUGIN_API UTunnelSectionCenterlineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 从 UDynamicMesh 基于截面质心生成隧道中心样条
	 *
	 * @param OwnerActor            样条附加的 Actor
	 * @param DynamicMesh           源动态网格
	 * @param MeshComponent         网格组件
	 * @param StepDistance          截面步距
	 * @param MaxSteps              最大步数
	 * @param InitialUseCustom      是否使用自定义起点/方向
	 * @param InitialOriginWorld    自定义起点
	 * @param InitialDirectionWorld 自定义初始方向
	 * @param SplineName            样条组件名称
	 * @param bReplaceExisting      复用同名样条并清空
	 * @param bUniformResample      是否等距重采样
	 * @param ResampleSpacing       等距重采样间距
	 * @param bDebugDraw            调试绘制
	 * @param DebugTime             调试持续时间
	 * @param bExtendToOpenings     是否自动延伸到隧道开口
	 * @param ExtensionStepRatio    延伸步距比例
	 * @param MaxExtensionSteps     最大延伸步数
	 * @return                      生成的 USplineComponent
	 */
	UFUNCTION(BlueprintCallable, Category="Tunnel|Centerline")
	static USplineComponent* BuildSimpleCenterSpline(
		AActor* OwnerActor,
		UDynamicMesh* DynamicMesh,
		USceneComponent* MeshComponent,
		float StepDistance = 100.f,
		int32 MaxSteps = 256,
		bool InitialUseCustom = false,
		FVector InitialOriginWorld = FVector::ZeroVector,
		FVector InitialDirectionWorld = FVector::ForwardVector,
		FName SplineName = TEXT("TunnelCenterSpline"),
		bool bReplaceExisting = true,
		bool bUniformResample = true,
		float ResampleSpacing = 100.f,
		bool bDebugDraw = false,
		float DebugTime = 5.f,
		bool bExtendToOpenings = true,
		float ExtensionStepRatio = 0.1f,
		int32 MaxExtensionSteps = 50
	);

private:
	static bool ExtractMeshCompact(UDynamicMesh* DynMesh, TArray<FVector>& OutPos, TArray<FIntVector>& OutTris);

	static FVector ComputePCA(const TArray<FVector>& Pos);

	static bool ComputePlaneSectionCentroid(
		const TArray<FVector>& Pos,
		const TArray<FIntVector>& Tris,
		const FVector& Origin,
		const FVector& Normal,
		FVector& OutCentroid);

	static TArray<FVector> TraceCentroids(
		const TArray<FVector>& Pos,
		const TArray<FIntVector>& Tris,
		const FBox& Bounds,
		FVector Origin,
		FVector Dir,
		float Step,
		int32 MaxSteps);

	static bool ExtendToOpening(
		const TArray<FVector>& Pos,
		const TArray<FIntVector>& Tris,
		FVector StartCentroid,
		FVector Direction,
		float StepSize,
		int32 MaxSteps,
		TArray<FVector>& OutExtensionPoints);

	static void ResampleUniform(const TArray<FVector>& In, float Spacing, TArray<FVector>& Out);

	static USplineComponent* CreateOrFindSpline(AActor* Owner, FName Name, bool bReplace);

	static void WriteWorldToSpline(USplineComponent* Spline, const TArray<FVector>& Pts);

	static void DrawDebugPolyline(UWorld* World, const TArray<FVector>& Pts, FColor Color, float Time);

	static int32 ComputeSectionIntersectionCount(
		const TArray<FVector>& Pos,
		const TArray<FIntVector>& Tris,
		const FVector& Origin,
		const FVector& Normal);

	static int32 ExtendToOpeningAggressive(
		const TArray<FVector>& Pos,
		const TArray<FIntVector>& Tris,
		const FBox& Bounds,
		FVector StartCentroid,
		FVector Direction,
		float StepSize,
		int32 MaxSteps,
		TArray<FVector>& OutExtensionPoints);
};
