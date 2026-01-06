#pragma once

#include "CoreMinimal.h"
#include "InTunnelSpline.h"
#include "GameFramework/Actor.h"
#include "InTunnelManager.generated.h"

class ABaseInTunnelActor;

/**
 * FTunnelSplineInfo
 * 安全步距功能生成所需位置样条线信息
 */
USTRUCT(BlueprintType)
struct FTunnelSplineInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FString TunnelName = TEXT("");
	UPROPERTY(BlueprintReadWrite)
	AInTunnelSpline* TunnelSpline = nullptr;
};

/**
 * FInTunnelInfo
 * 安全步距功能生成所需位置样条线信息
 */
USTRUCT(BlueprintType)
struct FInTunnelInfo
{
	GENERATED_BODY()
	
	// 名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InTunnelInfo")
	FString Name = FString();
	// 桩号
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InTunnelInfo")
	FString PileNumber = FString();
	// 当前位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InTunnelInfo")
	FString CurrLocation = FString();
	// 更新时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InTunnelInfo")
	FString UpdateTime = FString();
	

	FInTunnelInfo() = default;
	//
	// bool IsPerson() const { return Name == EInTunnelInfoType::Person; }
	// bool IsVehicle() const { return InfoType == EInTunnelInfoType::Vehicle; }
	
};


UCLASS()
class ZHJZPLUGIN_API AInTunnelManager : public AActor
{
	GENERATED_BODY()

public:
	AInTunnelManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;
	
public:
	UPROPERTY(EditAnywhere, Category="InTunnelManager")
	TSubclassOf<ABaseInTunnelActor> InTunnelVehicleClass;
	UFUNCTION(BlueprintCallable, Category = "InTunnelManager")
	FTunnelSplineInfo GetTunnelInfo(const FString& TargetTunnelName);
protected:
	UPROPERTY(EditAnywhere, Category = "InTunnelManager")
	TMap<FString, FTunnelSplineInfo> TunnelSplineMap;
	
	ABaseInTunnelActor* SpawnTunnelActor(const FInTunnelInfo& Info);
	void DestroyTunnelActor();
};
