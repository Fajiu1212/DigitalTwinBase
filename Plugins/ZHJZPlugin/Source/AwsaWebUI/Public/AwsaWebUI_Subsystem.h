#pragma once

#include "CoreMinimal.h"
#include "AutoParse.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AwsaWebUI_Subsystem.generated.h"

struct FJsonLibraryValue;
class UAwsaWebUI_Manager;

USTRUCT(BlueprintType)
struct FEnvUrl : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Mapping")
	FGameplayTag UrlTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Mapping")
	FString UrlString;
};

USTRUCT(BlueprintType)
struct FConditionTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Mapping")
	FString Condition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Mapping")
	bool bCondition;
};

#pragma region TuneelExcavationProgress Parsing

// 上下游方向 (原大||小里程方向)
UENUM(BlueprintType)
enum class ETunnelMileageDirection : uint8
{
	Up UMETA(DisplayName = "上游方向"),
	Down UMETA(DisplayName = "下游方向"),
	None UMETA(DisplayName = "未指定")
};

// 隧道左右幅
UENUM(BlueprintType)
enum class ETunnelSide : uint8
{
	Default UMETA(DisplayName = "DEFAULT"), // Layer1 only
	Left UMETA(DisplayName = "左幅"),
	Right UMETA(DisplayName = "右幅"),
	None UMETA(DisplayName = "未指定")
};

/**
 * Single work face info
 */
USTRUCT(BlueprintType)
struct FTunnelWorkFaceInfo
{
	GENERATED_BODY()

	// 所属隧洞
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Tunnel")
	FString OwnerTunnelName = TEXT("");

	// 所属层级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	int32 OwnerLayer = 1;

	// 所属左右幅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	ETunnelSide OwnerSide = ETunnelSide::Default;

	// 工作面名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	FString WorkFaceName = TEXT("");

	// 工作面里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	float Mileage = 0.0f;

	FTunnelWorkFaceInfo() = default;

	FTunnelWorkFaceInfo(const FString& InOwnerTunnelName, int32 InOwnerLayer, ETunnelSide InOwnerSide,
	                    const FString& InWorkFaceName, float InMileage)
		: OwnerTunnelName(InOwnerTunnelName),
		  OwnerLayer(InOwnerLayer),
		  OwnerSide(InOwnerSide),
		  WorkFaceName(InWorkFaceName),
		  Mileage(InMileage)
	{
	}

	bool operator==(const FTunnelWorkFaceInfo& Other) const
	{
		return OwnerTunnelName.Equals(Other.OwnerTunnelName, ESearchCase::IgnoreCase) &&
			OwnerLayer == Other.OwnerLayer &&
			OwnerSide == Other.OwnerSide &&
			WorkFaceName.Equals(Other.WorkFaceName, ESearchCase::IgnoreCase);
	}

	bool operator!=(const FTunnelWorkFaceInfo& Other) const
	{
		return !(*this == Other);
	}

	bool operator<(const FTunnelWorkFaceInfo& Other) const
	{
		return Mileage < Other.Mileage;
	}

	// 匹配指定工作面名称
	bool MatchesName(const FString& InName) const
	{
		return WorkFaceName.Equals(InName, ESearchCase::IgnoreCase);
	}

	bool IsValid() const
	{
		return !WorkFaceName.IsEmpty();
	}
};

/**
 * 模型区段信息
 * 存储 tunnelName + layer + 左右幅 + 起始/终止/工作面
 */
USTRUCT(BlueprintType)
struct FTunnelWorkFaceSection
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	FString TunnelName;

	// 层级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel", meta = (ClampMin = "1", ClampMax = "3"))
	int32 Layer = 1;

	// 左右幅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	ETunnelSide Side = ETunnelSide::Default;

	// 所有工作面列表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	TArray<FTunnelWorkFaceInfo> WorkFaces;

	// 默认构造
	FTunnelWorkFaceSection() = default;

	FTunnelWorkFaceSection(const FString& TunnelName, int32 Layer, ETunnelSide Side)
		: TunnelName(TunnelName),
		  Layer(Layer),
		  Side(Side)
	{
	}

	// 唯一键
	FString GetUniqueKey() const
	{
		FString SideStr = (Side == ETunnelSide::Left)
			                  ? TEXT("左幅")
			                  : (Side == ETunnelSide::Right)
			                  ? TEXT("右幅")
			                  : TEXT("DEFAULT");
		return FString::Printf(TEXT("%s_L%d_%s"), *TunnelName, Layer, *SideStr);
	}

	// 添加工作面（按里程升序排序）
	bool AddWorkFace(const FTunnelWorkFaceInfo& WorkFace)
	{
		WorkFaces.Add(WorkFace);
		WorkFaces.Sort();
		return true;
	}

	// 获取里程 by WorkFaceName
	bool GetMileageByName(const FString& WorkFaceName, float& OutMileage) const
	{
		for (const FTunnelWorkFaceInfo& WF : WorkFaces)
		{
			if (WF.MatchesName(WorkFaceName))
			{
				OutMileage = WF.Mileage;
				return true;
			}
		}
		return false;
	}

	// 计算两个工作面之间的距离（绝对值）
	bool CalculateDistance(const FString& FromWorkFace, const FString& ToWorkFace, float& OutDistance) const
	{
		float FromMileage = 0.0f;
		float ToMileage = 0.0f;

		if (!GetMileageByName(FromWorkFace, FromMileage))
		{
			return false;
		}

		if (!GetMileageByName(ToWorkFace, ToMileage))
		{
			return false;
		}

		OutDistance = FMath::Abs(ToMileage - FromMileage);
		return true;
	}

	// 获取所有工作面名称列表
	TArray<FString> GetAllWorkFaceNames() const
	{
		TArray<FString> Names;
		for (const FTunnelWorkFaceInfo& WF : WorkFaces)
		{
			Names.Add(WF.WorkFaceName);
		}
		return Names;
	}

	// 获取总里程 (弃用）
	float GetTotalMileage() const
	{
		if (WorkFaces.Num() == 0)
		{
			return 0.0f;
		}
		return WorkFaces.Last().Mileage - WorkFaces[0].Mileage;
	}

	// 检查是否有效
	bool IsValid() const
	{
		return !TunnelName.IsEmpty() && Layer >= 1 && Layer <= 3 && WorkFaces.Num() > 0;
	}
};


/**
 * 累计完成里程
 * 存储 tunnelName + layer + 左右幅 + 方向 -> 里程
 */
USTRUCT(BlueprintType)
struct FTunnelMileageProgress
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	FString TunnelName;

	// 层级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel", meta = (ClampMin = "1", ClampMax = "3"))
	int32 Layer = 1;

	// 左右幅
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	ETunnelSide Side = ETunnelSide::Default;

	// 里程方向
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	ETunnelMileageDirection Direction = ETunnelMileageDirection::None;

	// 工作面
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	FString WorkFaceName = TEXT("");

	// 累计完成里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	float CompletedMileage = 0.0f;

	// 今日里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	float TodayMileage = 0.0f;
	
	FTunnelMileageProgress() = default;

	FTunnelMileageProgress(const FString& InTunnelName, int32 InLayer, ETunnelSide InSide,
	                       ETunnelMileageDirection InDirection, const FString& InWorkFaceName, float InMileage,
	                       float InTodayMileage)
		: TunnelName(InTunnelName)
		  , Layer(InLayer)
		  , Side(InSide)
		  , Direction(InDirection)
		  , WorkFaceName(InWorkFaceName)
		  , CompletedMileage(InMileage)
		  , TodayMileage(InTodayMileage)
	{
	}

	// 唯一键
	FString GetUniqueKey() const
	{
		FString SideStr = (Side == ETunnelSide::Left)
			                  ? TEXT("左幅")
			                  : (Side == ETunnelSide::Right)
			                  ? TEXT("右幅")
			                  : TEXT("DEFAULT");
		FString DirStr = (Direction == ETunnelMileageDirection::Up)
			                 ? TEXT("上游方向")
			                 : (Direction == ETunnelMileageDirection::Down)
			                 ? TEXT("下游方向")
			                 : TEXT("未指定");
		return FString::Printf(TEXT("%s_L%d_%s_%s_%s"), *TunnelName, Layer, *SideStr, *DirStr, *WorkFaceName);
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty() && Layer >= 1 && Layer <= 3 &&
			Direction != ETunnelMileageDirection::None && !WorkFaceName.IsEmpty();
	}
};

/**
 * Single tunnel floor progress
 * 所属隧洞 + 起点里程 + 终点里程
 */
USTRUCT(BlueprintType)
struct FSingleTunnelFloorInfo
{
	GENERATED_BODY()

	// 所属隧洞
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Tunnel")
	FString OwnerTunnelName = TEXT("");

	// 起点里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	float BeginMileage = 0.f;

	// 终点里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	float EndMileage = 0.f;

	FSingleTunnelFloorInfo() = default;

	FSingleTunnelFloorInfo(const FString& OwnerTunnelName, float BeginMileage, float EndMileage)
		: OwnerTunnelName(OwnerTunnelName),
		  BeginMileage(BeginMileage),
		  EndMileage(EndMileage)
	{
	}

	bool operator==(const FSingleTunnelFloorInfo& Other) const
	{
		return OwnerTunnelName.Equals(Other.OwnerTunnelName, ESearchCase::IgnoreCase) &&
			BeginMileage == Other.BeginMileage &&
			EndMileage == Other.EndMileage;
	}

	bool operator!=(const FSingleTunnelFloorInfo& Other) const
	{
		return !(*this == Other);
	}

	bool operator<(const FSingleTunnelFloorInfo& Other) const
	{
		return BeginMileage < Other.BeginMileage;
	}

	bool IsValid() const
	{
		return !OwnerTunnelName.IsEmpty();
	}
};

/**
 * Tunnel WorkFloorInfo
 * 存储 tunnelName + layer + 左右幅 + 起始/终止/工作面
 */
USTRUCT(BlueprintType)
struct FTunnelCompletedFloors
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	FString TunnelName;

	// 隧洞总长
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	float TotalTunnelLength = 0.f;

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel")
	TArray<FSingleTunnelFloorInfo> CompletedFloorInfos;

	FTunnelCompletedFloors() = default;

	FTunnelCompletedFloors(const FString& TunnelName, float TotalTunnelLength)
		: TunnelName(TunnelName),
		  TotalTunnelLength(TotalTunnelLength)
	{
	}

	// 添加已完成底板
	bool AddCompletedFloor(const FSingleTunnelFloorInfo& NewCompletedFloor)
	{
		if (NewCompletedFloor.IsValid())
		{
			CompletedFloorInfos.Add(NewCompletedFloor);
			CompletedFloorInfos.Sort();
			return true;
		}
		return false;
	}

	bool operator ==(const FTunnelCompletedFloors& Other) const
	{
		return TunnelName.Equals(Other.TunnelName, ESearchCase::IgnoreCase);
	}

	bool operator !=(const FTunnelCompletedFloors& Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty();
	}
};

/**
 * Single geological forecast
 * 所属隧洞 + 起点里程 + 终点里程 + 预告级别
 */
USTRUCT(BlueprintType)
struct FSingleGeologicalForecast
{
	GENERATED_BODY()

	// 所属隧洞
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "GeologicalForecast")
	FString OwnerTunnelName = TEXT("");

	// 起点里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeologicalForecast")
	float BeginMileage = 0.f;

	// 终点里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeologicalForecast")
	float EndMileage = 0.f;

	// 预告级别
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeologicalForecast")
	int32 LevelIndex = 0;

	// id（点击时传给前端）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "GeologicalForecast")
	int32 ID = 0;

	FSingleGeologicalForecast() = default;

	FSingleGeologicalForecast(const FString& OwnerTunnelName, float BeginMileage, float EndMileage, int32 LevelIndex,
	                          int32 InId)
		: OwnerTunnelName(OwnerTunnelName),
		  BeginMileage(BeginMileage),
		  EndMileage(EndMileage),
		  LevelIndex(LevelIndex),
		  ID(InId)
	{
	}

	bool operator==(const FSingleGeologicalForecast& Other) const
	{
		return OwnerTunnelName.Equals(Other.OwnerTunnelName, ESearchCase::IgnoreCase) &&
			BeginMileage == Other.BeginMileage &&
			EndMileage == Other.EndMileage &&
			LevelIndex == Other.LevelIndex &&
			ID == Other.ID;
	}

	bool operator!=(const FSingleGeologicalForecast& Other) const
	{
		return !(*this == Other);
	}

	bool operator<(const FSingleGeologicalForecast& Other) const
	{
		return BeginMileage < Other.BeginMileage;
	}

	bool IsValid() const
	{
		return !OwnerTunnelName.IsEmpty();
	}
};

/**
 * GeologicalForecastIndo
 * 存储超前地质预告色块信息
 */
USTRUCT(BlueprintType)
struct FGeologicalForecastInfo
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeologicalForecast")
	FString TunnelName;

	// 地质预告数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GeologicalForecast")
	TArray<FSingleGeologicalForecast> GeologicalForecastArr;

	FGeologicalForecastInfo() = default;

	explicit FGeologicalForecastInfo(const FString& TunnelName)
		: TunnelName(TunnelName)
	{
	}

	// 添加单个地质预告
	bool AddSingleGeologicalForecast(const FSingleGeologicalForecast& NewSingleGeologicalForecast)
	{
		if (NewSingleGeologicalForecast.IsValid())
		{
			GeologicalForecastArr.Add(NewSingleGeologicalForecast);
			GeologicalForecastArr.Sort();
			return true;
		}
		return false;
	}

	bool operator ==(const FGeologicalForecastInfo& Other) const
	{
		return TunnelName.Equals(Other.TunnelName, ESearchCase::IgnoreCase);
	}

	bool operator !=(const FGeologicalForecastInfo& Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty();
	}
};

/**
 * UnderControlActors
 * 剖面关卡蓝图存储需要显影的场景Actor
 */
USTRUCT(BlueprintType)
struct FUnderControlActors
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnderControlActors")
	FString TunnelName;

	// 需要显影的Actor名称数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnderControlActors")
	TArray<AActor*> Actors;

	FUnderControlActors() = default;

	FUnderControlActors(const FString& TunnelName, const TArray<AActor*>& Actors)
		: TunnelName(TunnelName),
		  Actors(Actors)
	{
	}

	bool operator ==(const FUnderControlActors& Other) const
	{
		return TunnelName.Equals(Other.TunnelName, ESearchCase::IgnoreCase);
	}

	bool operator !=(const FUnderControlActors& Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty();
	}
};

/**
 * Single Over Excavation Info
 * 单个超限挖切片信息
 */
USTRUCT(BlueprintType)
struct FSingleOverExcavationInfo
{
	GENERATED_BODY()

	// 所属隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleOverExcavationInfo")
	FString OwnerTunnelName;
	// 桩号
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleOverExcavationInfo")
	FString Stake = TEXT("");
	// 里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleOverExcavationInfo")
	float Mile = 0.f;
	// 是否为重要
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleOverExcavationInfo")
	bool bImportant;

	FSingleOverExcavationInfo() = default;

	FSingleOverExcavationInfo(const FString& TunnelName, const FString& Stake, float Mile, bool bImportant)
		: OwnerTunnelName(TunnelName),
		  Stake(Stake),
		  Mile(Mile),
		  bImportant(bImportant)
	{
	}

	bool operator==(const FSingleOverExcavationInfo& Other) const
	{
		return OwnerTunnelName.Equals(Other.OwnerTunnelName, ESearchCase::IgnoreCase) &&
			Stake == Other.Stake &&
			Mile == Other.Mile &&
			bImportant == Other.bImportant;
	}

	bool operator!=(const FSingleOverExcavationInfo& Other) const
	{
		return !(*this == Other);
	}

	bool operator<(const FSingleOverExcavationInfo& Other) const
	{
		return Mile < Other.Mile;
	}

	bool IsValid() const
	{
		return !OwnerTunnelName.IsEmpty();
	}
};

/**
 * 超欠挖
 * 存储 隧洞名称 + 单个切片数组
 */
USTRUCT(BlueprintType)
struct FTunnelOverExcavationInfo
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTunnelOverExcavationInfo")
	FString TunnelName;
	// 切片数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTunnelOverExcavationInfo")
	TArray<FSingleOverExcavationInfo> SingleOverExcavationInfoArr;


	FTunnelOverExcavationInfo() = default;

	FTunnelOverExcavationInfo(const FString& TunnelName)
		: TunnelName(TunnelName)
	{
	}


	// 添加切片
	bool AddSlide(const FSingleOverExcavationInfo& NewSlide)
	{
		SingleOverExcavationInfoArr.Add(NewSlide);
		SingleOverExcavationInfoArr.Sort();
		return true;
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty();
	}
};

/**
 * Single surface
 * 单个围岩
 */
USTRUCT(BlueprintType)
struct FSingleSurface
{
	GENERATED_BODY()


	// 所属隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleSurface")
	FString OwnerTunnelName;
	// 桩号
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleSurface")
	FString Stake = TEXT("");
	// 里程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleSurface")
	float Mile = 0.f;
	// 是否为重要
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleSurface")
	bool bImportant = false;

	FSingleSurface() = default;

	FSingleSurface(const FString& OwnerTunnelName, const FString& Stake, float Mile, bool bImportant)
		: OwnerTunnelName(OwnerTunnelName),
		  Stake(Stake),
		  Mile(Mile),
		  bImportant(bImportant)
	{
	}

	bool operator==(const FSingleSurface& Other) const
	{
		return OwnerTunnelName.Equals(Other.OwnerTunnelName, ESearchCase::IgnoreCase) &&
			Stake == Other.Stake &&
			Mile == Other.Mile &&
			bImportant == Other.bImportant;
	}

	bool operator!=(const FSingleSurface& Other) const
	{
		return !(*this == Other);
	}

	bool operator<(const FSingleSurface& Other) const
	{
		return Mile < Other.Mile;
	}

	bool IsValid() const
	{
		return !OwnerTunnelName.IsEmpty();
	}
};

/**
 * Single surface
 * 围岩变形监测
 */
USTRUCT(BlueprintType)
struct FSurfaceInfo
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTunnelOverExcavationInfo")
	FString TunnelName;
	// 断面数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTunnelOverExcavationInfo")
	TArray<FSingleSurface> SingleSurfaceArr;


	FSurfaceInfo() = default;

	explicit FSurfaceInfo(const FString& TunnelName)
		: TunnelName(TunnelName)
	{
	}

	// 添加断面
	bool AddSurface(const FSingleSurface& NewSlide)
	{
		SingleSurfaceArr.Add(NewSlide);
		SingleSurfaceArr.Sort();
		return true;
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty();
	}
};

/**
 * Vehicle type
 * 车辆类型 油、电（目前只有两个）
 */
UENUM(BlueprintType)
enum class EVehicleType : uint8
{
	Petrol UMETA(DisplayName = "油车"),
	Electric UMETA(DisplayName = "电车"),
	None UMETA(DisplayName = "未指定")
};

/**
 * Single vehicle
 * 单个vehicle info
 */
USTRUCT(BlueprintType)
struct FSingleVehicle
{
	GENERATED_BODY()
	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleVehicle")
	FString TunnelName;
	// 桩号
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleVehicle")
	FString Stake = TEXT("");
	// 车牌号
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleVehicle")
	FString LicensePlateNumber = TEXT("");
	// 距离洞口（米）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleVehicle")
	FString Distance = TEXT("");

	// 车辆ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleVehicle")
	int32 CarId = 0;

	// 车辆类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleVehicle")
	EVehicleType VehicleType = EVehicleType::None;
	// 是否在洞
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSingleVehicle")
	bool bIsInHole = false;

	FSingleVehicle() = default;

	FSingleVehicle(const FString& TunnelName, const FString& Stake, const FString& LicensePlateNumber,
	               const FString& Distance, int32 CarId, EVehicleType VehicleType, bool bIsInHole)
		: TunnelName(TunnelName),
		  Stake(Stake),
		  LicensePlateNumber(LicensePlateNumber),
		  Distance(Distance),
		  CarId(CarId),
		  VehicleType(VehicleType),
		  bIsInHole(bIsInHole)
	{
	}

	bool operator==(const FSingleVehicle& Other) const
	{
		return TunnelName == Other.TunnelName &&
			Stake == Other.Stake &&
			LicensePlateNumber == Other.LicensePlateNumber &&
			Distance == Other.Distance &&
			CarId == Other.CarId &&
			VehicleType == Other.VehicleType &&
			bIsInHole == Other.bIsInHole;
	}

	bool operator!=(const FSingleVehicle& Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty() && !LicensePlateNumber.IsEmpty();
	}
};

/**
 * Tunnel Vehicle Info
 * 隧洞车辆存储 隧洞名称 + 车辆数组
 */
USTRUCT(BlueprintType)
struct FTunnelVehicleInfo
{
	GENERATED_BODY()

	// 隧洞名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTunnelVehicleInfo")
	FString TunnelName;
	// 车辆数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FTunnelVehicleInfo")
	TArray<FSingleVehicle> SingleVehicleArr;

	FTunnelVehicleInfo() = default;

	explicit FTunnelVehicleInfo(const FString& TunnelName)
		: TunnelName(TunnelName)
	{
	}

	// 添加车辆
	bool AddVehicle(const FSingleVehicle& NewVehicle)
	{
		SingleVehicleArr.Add(NewVehicle);
		return true;
	}

	bool IsValid() const
	{
		return !TunnelName.IsEmpty();
	}
};

#pragma endregion

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTestEnvHotLoad);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCallWebFunc, const FString&, Function, const FJsonLibraryValue&, Data);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNativeSelectionChanged, int32, Index);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTogglePOIActor, FString, Teye, bool, bShow);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressLoadSuccess, const FString&, TargetModuleName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlabLoadSuccess, int32, InSlabIndex, const FString&, TargetModuleName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeologicalForecastLoadSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnModuleSwitched, const FString&, TargetModuleName, const FString&,
                                             ExtraParam);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExplosionAnimation, const FString&, TargetOwnerTunnelName, bool,
                                             bResetCamera);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSectionCuttingSelected, const FString&, TargetTunnelName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOverExcavationLoadSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSurfaceInfoLoadSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTunnelVehicleLoadSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniMapToggleClicked, bool, bIsShow);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiniMapIconClicked, const FString&, LocStr);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonitorOverviewPageChanged, int32, TargetPageIndex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonitorClosed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCallDisableWebUI, bool, bDisable);
UCLASS(Config=Toolkits)
class AWSAWEBUI_API UAwsaWebUI_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UAwsaWebUI_Subsystem();
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//FOnCallWebFunc
public:
	UPROPERTY(BlueprintAssignable)
	FOnCallWebFunc OnCallWebFunc;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallWebFunc(const FString& Function, const FJsonLibraryValue& Data);

	// Diable web ui
	UPROPERTY()
	FOnCallDisableWebUI OnCallDisableWebUI;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallDiableWebUI(bool bDisable);
	//Token
private:
	FString Token = TEXT("");

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Toolkits|WebUI")
	FString GetToken()
	{
		return Token;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Toolkits|WebUI")
	FString GetToken_UrlAppend()
	{
		return TEXT("?token=") + Token;
	}

	//orgid
private:
	FString ConfigOrgId;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AwsaWebUI|Subsystem")
	FString GetOrgId()
	{
		return ConfigOrgId;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AwsaWebUI|Subsystem")
	FString GetOrgId_UrlAppend()
	{
		return TEXT("&oigId=") + ConfigOrgId;
	}

	//Url_Pre
private:
	UPROPERTY(Config)
	FString Url_Pre;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AwsaWebUI|Subsystem")
	FString GetUrl_Pre()
	{
		return Url_Pre;
	}

	//ImgUrl_Pre
private:
	UPROPERTY(Config)
	FString ImgUrl_Pre;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AwsaWebUI|Subsystem")
	FString GetImgUrl_Pre()
	{
		return ImgUrl_Pre;
	}

private:
	void TokenRequest();
	UFUNCTION()
	void SaveTokenRequest(const FJsonStruct& JsonStruct, const FString&
	                      WebMsg, const FString& StructName);
	// Production env
	UPROPERTY()
	UDataTable* EnvUrlDataTable;
	static const FString EnvUrlDataTablePath;
	UPROPERTY()
	TArray<FEnvUrl> EnvUrlRows;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AwsaWebUI|Env")
	FString GetEnvUrl(FGameplayTag UrlTag);
	UFUNCTION(BlueprintCallable, Category="AwsaWebUI|Env")
	void UpdateVersion(FString NewVersion);
	// Condition
private:
	UPROPERTY()
	UDataTable* ConditionDataTable;
	static const FString ConditionDataTablePath;

public:
	bool CheckTestEnv();
	UFUNCTION(BlueprintCallable, Category = "AwsaGameInstance|Condition")
	bool CheckCondition(const FString& Condition);

private:
	//Manager
	UPROPERTY()
	TObjectPtr<UAwsaWebUI_Manager> WebUIManager = nullptr;
	void CreateWebUIManager();

public:
	UFUNCTION(BlueprintCallable, Category = "Toolkits|WebUI")
	UAwsaWebUI_Manager* GetWebUIManager() const { return WebUIManager; }

	//Listeners
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RegisterListener(UObject* Listener);
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void UnregisterListener(UObject* Listener);

	void BroadcastManager(const FString& Name, const FString& Message);
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void BroadcastMessage(const FString& Name, const FString& Message, const FGameplayTag& RequiredTag);

private:
	TArray<TWeakObjectPtr<UObject>> Listeners;

	// HotLoad
	UFUNCTION(BlueprintCallable, Category="Toolkits|Webui")
	void OnHotLoadSuccess(const FString& OldPakFile);

public:
	UPROPERTY(BlueprintAssignable, Category="Toolkits|WebUI")
	FOnTestEnvHotLoad OnTestEnvHotLoad;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnTestEnvHotLoad();

	// Center popup window
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void OpenCenterPopupWindow(const FString& InComponentStr, const FString& InParamsObjStr);

private:
	void DeleteWebCache();

	UPROPERTY(BlueprintAssignable)
	FOnNativeSelectionChanged OnNativeSelectionChanged;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnNativeSelectionChanged(int32 Index);

public:
	// 模式切换
	UPROPERTY(BlueprintAssignable)
	FOnModuleSwitched ModuleSwitched;
	UFUNCTION(BlueprintCallable, Category="Toolkits|Webui")
	void CallModuleSwitched(const FString& InModuleName, const FString& InExtraParam);

	UPROPERTY(BlueprintAssignable, Category="Toolkits|WebUI")
	FOnTogglePOIActor OnTogglePoiActor;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnTogglePoiActor(FString TargetType, bool bShow);

	// Progress

	// 开挖进度
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RequestListProgress(const FString& TargetModuleName);
	UFUNCTION()
	void OnProgressListSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg, const FString& StructName);
	UFUNCTION()
	void UpdateProgressList();
	// TunnelWorkFaceSectionArr
	UPROPERTY()
	TArray<FTunnelWorkFaceSection> TunnelWorkFaceSectionArr;
	FTunnelWorkFaceSection* FindSectionPtr(const FString& TargetTunnelName, int32 TargetLayer, ETunnelSide TargetSide);
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	bool CalculateTunnelModelDistance(const FString& TunnelName, int32 Layer, ETunnelSide Side,
	                                  const FString& BeginWorkFace, const FString& EndWorkFace, float& OutDistance);
	UPROPERTY(BlueprintAssignable)
	FOnProgressLoadSuccess OnProgressLoadSuccess;
	// Tunnel mileage progress array
	UPROPERTY()
	TArray<FTunnelMileageProgress> TunnelMileageProgressArr;
	bool FindMileageProgressPtr(const FString& TargetTunnelName, int32 TargetLayer,
	                            ETunnelSide TargetSide, ETunnelMileageDirection TargetDirection,
	                            const FString& TargetWorkFaceName,
	                            float& OutCompetedMileage, float& OutTodayMileage);
	// 顶拱、底板

	// Request http
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RequestSlab(const FString& TargetSlabType, const FString& TargetModuleName);
	UFUNCTION()
	void OnSlabRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg, const FString& StructName);
	// Delegate
	UPROPERTY(BlueprintAssignable)
	FOnSlabLoadSuccess OnSlabLoadSuccess;

	// Floors arr.
	UPROPERTY()
	TArray<FTunnelCompletedFloors> TunnelCompletedFloorsArr;
	// Find target floors arr.
	bool FindCompletedFloorsByTunnelName(const FString& TargetTunnelName,
	                                     TArray<FSingleTunnelFloorInfo>& OutCompletedFloors,
	                                     float& OutTotalTunnelLength);

	// 超前地质预报
	UPROPERTY()
	TMap<FString, FGeologicalForecastInfo> GeologicalForecastInfos;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RequestGeologicalForecast();
	UFUNCTION()
	void OnGeologicalForecastRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
	                                        const FString& StructName);
	UPROPERTY(BlueprintAssignable)
	FOnGeologicalForecastLoadSuccess OnGeologicalForecastLoadSuccess;
	bool FindGeologicalForecastByTunnelName(const FString& TunnelName,
	                                        FGeologicalForecastInfo& OutSingleGeologicalForecasts);

	// 隧道超欠挖
	TArray<FTunnelOverExcavationInfo> TunnelOverExcavationInfoArr;
	bool FindSlidesArrByTunnelName(const FString& TargetTunnelName,
	                               TArray<FSingleOverExcavationInfo>& TargetSlidesArr);

	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RequestTunnelOverExcavation();
	UFUNCTION()
	void OnTunnelOverExcavationRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
	                                          const FString& StructName);
	// Delegate
	UPROPERTY(BlueprintAssignable)
	FOnOverExcavationLoadSuccess OnOverExcavationLoadSuccess;

	// 隧道剖切
	UPROPERTY(BlueprintAssignable)
	FOnSectionCuttingSelected OnSectionCuttingSelected;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnSectionCuttingSelected(const FString& TargetTunnelName);
	// 展开动画
	UPROPERTY(BlueprintAssignable)
	FOnExplosionAnimation OnExplosionAnimation;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnExplosionAnimation(const FString& TargetOwnerTunnelName,bool bResetCamera);

	// 围岩变形
	TArray<FSurfaceInfo> TunnelSurfaceInfoArr;
	bool FindSurfaceArrByTunnelName(const FString& TargetTunnelName,
	                                TArray<FSingleSurface>& OutSingleSurfaceArr);

	UPROPERTY(BlueprintAssignable)
	FOnSurfaceInfoLoadSuccess OnSurfaceInfoLoadSuccess;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RequestTunnelSurfaceInfo();
	UFUNCTION()
	void OnTunnelSurfaceInfoRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
	                                       const FString& StructName);

	// 车辆定位查询
	// 移至BaseTunnelGenerator代理中，由隧道生成器统一管理()
	// UFUNCTION(BlueprintCallable)
	// void RequestSurfaceInfoRequestSuccess(const FString& OwnerTunnelName, const FJsonStruct& JsonStruct, const FString& StructName);
	// UPROPERTY() %LOCALAPPDATA%\GS_HydropowerPro\Saved

	TArray<FTunnelVehicleInfo> TunnelVehicleInfoArr;
	bool FindTunnelVehicleInfoByTunnelName(const FString& TunnelName,
	                                       FTunnelVehicleInfo& OutTunnelVehicleInfo);

	UPROPERTY(BlueprintAssignable)
	FOnTunnelVehicleLoadSuccess OnTunnelVehicleLoadSuccess;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RequestTunnelVehicle();
	UFUNCTION()
	void OnTunnelVehicleRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
	                                   const FString& StructName);

	// Minimap
	UPROPERTY(BlueprintAssignable)
	FOnMiniMapToggleClicked OnMiniMapToggleClicked;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnMinimapToggleClicked(bool bShowMap);
	UPROPERTY(BlueprintAssignable)
	FOnMiniMapIconClicked OnMiniMapIconClicked;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnMiniMapIconClicked(const FString& TargetLocStr);

	// Overview monitor page del
	UPROPERTY(BlueprintAssignable, Category="Toolkits|WebUI")
	FOnMonitorOverviewPageChanged OnMonitorOverviewPageChanged;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnMonitorOverviewPageChanged(int32 TargetPageIndex);

	UPROPERTY(BlueprintAssignable, Category="Toolkits|WebUI")
	FOnMonitorClosed OnMonitorClosed;
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void CallOnMonitorClosed();
	
	// // Subsystem
	// UFUNCTION(BlueprintPure, Category="Toolkits|WebUI")
	// static FString UrlEncode(const FString& SourceStr);
};
