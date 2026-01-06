#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GameFramework/Actor.h"
#include "TunnelBase.generated.h"

UENUM()
enum class ETunnelLayerType : uint8
{
	SingleLayer,   // 单层
	MultiLayer     // 多层
};

UENUM()
enum class EExcavationDirection : uint8
{
	Left,   // 左侧
	Right    // 右侧
};

// 是否双向开挖
// UENUM()
// enum class ETunnel : uint8
// {
// 	
// };

USTRUCT()
struct FLayerProgress
{
	GENERATED_BODY()
	FLayerProgress(){}
	
	int32 LayerIndex;
	float ProgressPercent;
	float TotalAnimationLength;
	float CompletedAnimationLength;
	float RemainingAnimationLength;
};
// Struct
USTRUCT()
struct FTunnelInfo
{
	GENERATED_BODY()
	FTunnelInfo(){}

	FString TunnelName;              // 隧洞名称
	ETunnelLayerType LayerType;      // 分层类型
	int32 LayerIndex;                // 层索引（0=底层，1=中层，2=顶层）

	FString WorkSection;             // 工作面/部位标识
	EExcavationDirection Direction;  // 开挖方向
	
	float TotalLength;               // 总里程
	float CompletedLength;           // 已完成里程
};
UCLASS()
class ZHJZPLUGIN_API ATunnelBase : public  ADynamicMeshActor
{
	GENERATED_BODY()

public:
	ATunnelBase();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;
	
	// 源 StaticMesh（需开启 Allow CPU Access）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StaticToDynamic|Source")
	UStaticMesh* SourceStaticMesh = nullptr;

	// 是否在 BeginPlay 自动从 SourceStaticMesh 拷贝
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StaticToDynamic|Runtime")
	bool bBuildAtBeginPlay = true;

	// 是否把源网格的材质列表复制到 DynamicMeshComponent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StaticToDynamic|Runtime")
	bool bCopyMaterials = true;

	// 选择拷贝的 LOD（通常 0）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StaticToDynamic|Runtime", meta=(ClampMin="0"))
	int32 LODIndex = 0;
};
