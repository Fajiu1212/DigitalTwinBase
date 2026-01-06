#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BeamGeneratorSubsystem.generated.h"

class ABeamActorBase;
struct FJsonStruct;

USTRUCT(BlueprintType, meta=(Displayname="BeamStruct"))
struct FBeamStruct
{
	GENERATED_BODY()

	FBeamStruct()
	{
	}

	// AI Pedestal
	FBeamStruct(const int32 BeamAssetsId, const FString& BeamAssetsName, const FString& Area,
	            const FString& ProcedureName, const FString& RoiImage, const FString& IdentifyTime,
	            const FString& RelatedMonitoringName, const int32 BeamId, const FString& BeamCode,
	            const FString& BeamLineChildConfigName, const FString& PlannedBeginDate,
	            const FString& BeamMoldConfigName,
	            const FString& BeamSectionConfigName, const FString& BridgeName)
		: BeamAssetsId(BeamAssetsId),
		  BeamAssetsName(BeamAssetsName),
		  Area(Area),
		  ProcedureName(ProcedureName),
		  RoiImage(RoiImage),
		  IdentifyTime(IdentifyTime),
		  RelatedMonitoringName(RelatedMonitoringName),
		  BeamId(BeamId),
		  BeamCode(BeamCode),
		  BeamLineChildConfigName(BeamLineChildConfigName),
		  PlannedBeginDate(PlannedBeginDate),
		  BeamMoldConfigName(BeamMoldConfigName),
		  BeamSectionConfigName(BeamSectionConfigName),
		  BridgeName(BridgeName)
	{
	}

	// Gjbz
	FBeamStruct(const int32 BeamAssetsId, const FString& BeamAssetsName)
		: BeamAssetsId(BeamAssetsId),
		  BeamAssetsName(BeamAssetsName)
	{
	}

	// Zltz
	FBeamStruct(
	const FString& InBeamMoldConfigName, const FString& InBeamCode,
	const FString& InBeamBridgeConfigName, const FString& InBeamSectionConfigName, const FString& InProcedureName,
	const FString& InName, const FString& InBeamLineChildConfigName, int32 InAssetsId)
	: BeamAssetsId(InAssetsId),
	  BeamAssetsName(InName),
	  ProcedureName(InProcedureName),
	  BeamCode(InBeamCode),
	  BeamLineChildConfigName(InBeamLineChildConfigName),
	  BeamMoldConfigName(InBeamMoldConfigName),
	  BeamSectionConfigName(InBeamSectionConfigName),
	  BridgeName(InBeamBridgeConfigName)
	{
	}

	// Pedestal Params
	// 台座ID（唯一标识符）
	UPROPERTY(BlueprintReadWrite)
	int32 BeamAssetsId = -1;
	// 台座名称
	UPROPERTY(BlueprintReadWrite)
	FString BeamAssetsName = TEXT("");
	// 区域
	UPROPERTY(BlueprintReadWrite)
	FString Area = TEXT("");
	// 工序名称
	UPROPERTY(BlueprintReadWrite)
	FString ProcedureName = TEXT("");
	// 识别图像
	FString RoiImage = TEXT("");
	// 识别时间
	UPROPERTY(BlueprintReadWrite)
	FString IdentifyTime = TEXT("");
	// 关联Monitor
	UPROPERTY(BlueprintReadWrite)
	FString RelatedMonitoringName = TEXT("");

	//Beam Params
	// 梁id
	UPROPERTY(BlueprintReadWrite)
	int32 BeamId = -1;
	// 梁编号
	UPROPERTY(BlueprintReadWrite)
	FString BeamCode = TEXT("");
	// 子生产线
	UPROPERTY(BlueprintReadWrite)
	FString BeamLineChildConfigName = TEXT("");
	// 计划架设时间
	UPROPERTY(BlueprintReadWrite)
	FString PlannedBeginDate = TEXT("");
	// 梁型
	UPROPERTY(BlueprintReadWrite)
	FString BeamMoldConfigName = TEXT("");
	// 标段名称
	UPROPERTY(BlueprintReadWrite)
	FString BeamSectionConfigName = TEXT("");
	// 桥段名称
	UPROPERTY(BlueprintReadWrite)
	FString BridgeName = TEXT("");

	bool operator==(const FBeamStruct& Other) const
	{
		return this->BeamAssetsId == Other.BeamAssetsId;
	}

	static FBeamStruct MakeFinder(int32 TargetBeamId)
	{
		FBeamStruct Temp;
		Temp.BeamId = TargetBeamId;
		return Temp;
	}

	friend uint32 GetTypeHash(const FBeamStruct& Other)
	{
		return GetTypeHash(Other.BeamId);
	}
	bool IsExactlyEqual(const FBeamStruct& Other) const
	{
		return BeamAssetsId == Other.BeamAssetsId
			&& BeamAssetsName == Other.BeamAssetsName
			&& Area == Other.Area
			&& ProcedureName == Other.ProcedureName
			&& RoiImage == Other.RoiImage
			&& IdentifyTime == Other.IdentifyTime
			&& RelatedMonitoringName == Other.RelatedMonitoringName
			&& BeamId == Other.BeamId
			&& BeamCode == Other.BeamCode
			&& BeamLineChildConfigName == Other.BeamLineChildConfigName
			&& PlannedBeginDate == Other.PlannedBeginDate
			&& BeamMoldConfigName == Other.BeamMoldConfigName
			&& BeamSectionConfigName == Other.BeamSectionConfigName
			&& BridgeName == Other.BridgeName;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllBeamReset);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeamClicked);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAiModeChanged, bool, bIsAiMode);

UCLASS()
class ZHJZPLUGIN_API UBeamGeneratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void InitProduction(const FString& WebMsg);

public:
	// Delegates
	UPROPERTY(BlueprintAssignable, Category="BeamGeneratorSubsystem")
	FOnAllBeamReset OnAllBeamReset;
	// BP call func
	UFUNCTION(BlueprintCallable, Category = "BeamGeneratorSubsystem")
	void InitBeamGenerators();
	UFUNCTION(BlueprintCallable, Category = "BeamGeneratorSubsystem")
	void RequestProduction();

protected:
	// Ai production
	void RequestProduction_Ai();
	void ProcessingProductionData_Ai(const FString& WebMsg);
	void GenerateProduction_Ai();

	// Static pedestal
	void RequestProduction_Static();
	void ProcessingProductionData_Gjbz(const FString& WebMsg);
	void ProcessingProductionData_Zltz(const FString& WebMsg);
	void ProcessingProductionData_Cltz(const FString& WebMsg);
	void GenerateProduction_Gjbz();
	void GenerateProduction_Zltz();
	void GenerateProduction_Cltz();

	UFUNCTION()
	void OnRequestAiProductionComplete(const FJsonStruct& JsonStruct, const FString&
	                                   WebMsg, const FString& StructName);
	UFUNCTION()
	void OnRequestStaticProductionComplete(const FJsonStruct& JsonStruct, const FString&
	                                       WebMsg, const FString& StructName);

	UPROPERTY()
	TArray<ABeamActorBase*> AiPedestals;
	UPROPERTY()
	TMap<FString, FBeamStruct> AiPedestalsRequestMap;
	UPROPERTY()
	TMap<FString, FBeamStruct> NewAiPedestalsRequestMap;

	UPROPERTY()
	TArray<ABeamActorBase*> StaticPedestals;
	UPROPERTY()
	TMap<FString, FBeamStruct> StaticPedestalsRequestMap_Gjbz;
	UPROPERTY()
	TMap<FString, FBeamStruct> StaticPedestalsRequestMap_Zltz;

public:
	UPROPERTY(BlueprintReadWrite, Category="Beam")
	bool bAllowPoi = true;
};
