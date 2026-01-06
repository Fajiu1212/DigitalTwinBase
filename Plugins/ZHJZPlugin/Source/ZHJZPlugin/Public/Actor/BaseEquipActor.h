#pragma once

#include "CoreMinimal.h"
#include "AutoParse.h"
#include "AwsaWebUI_Interface.h"
#include "EquipActorSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "BaseEquipActor.generated.h"

class AAwsaPlayerPawn;
class UWidgetComponent;
class UAwsaWebUI_ActorDashboard;

UENUM(BlueprintType)
enum class EEquipActorType : uint8
{
	Default,
	Panel,
	Helmet,
	DustAndNoise,
	MonitorCamera,
	MonitorCamera_AI
};

USTRUCT(BlueprintType)
struct FUrlExtraParam
{
	GENERATED_BODY()

	FUrlExtraParam() = default;

	FUrlExtraParam(const FString& Key, const FString& Value)
		: Key(Key),
		  Value(Value)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Key;
	UPROPERTY(BlueprintType)
	FString Value;

	void Add(const FString& NewKey, const FString& NewValue)
	{
		if (!NewKey.IsEmpty())
		{
			Key = NewKey;
			Value = NewValue;
		}
	}
};

USTRUCT(BlueprintType)
struct FEquipUrl : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Mapping")
	FGameplayTag UrlTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Mapping")
	FString UrlString;
};

UCLASS()
class ZHJZPLUGIN_API ABaseEquipActor : public AActor, public IAwsaWebUI_Interface
{
	GENERATED_BODY()

public:
	ABaseEquipActor();
	virtual void PostInitializeComponents() override;
	virtual void ReceiveFunction_Implementation(const FString& Name, const FString& Message) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
private:
	// Components
	UPROPERTY()
	UStaticMeshComponent* MonitorMesh;
	UPROPERTY()
	UStaticMeshComponent* HandleMesh;
	UPROPERTY()
	UStaticMeshComponent* MonitorRangeMesh;

	static const FString MonitorMeshPath;
	static const FString HandleMeshPath;
	static const FString MonitorRangeMeshPath;

	void UpdateMonitorMeshVisibility(const bool bShowMonitor);

	// Params
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayPriority = 0, DisplayName = "Type", AllowPrivateAccess = "true"))
	EEquipActorType EquipActorType = EEquipActorType::Default;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "EquipName", AllowPrivateAccess = "true"))
	FString EquipName = FString(TEXT("MonitorEquip"));

	static const FString DefaultIconPath;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Intro Icon", AllowPrivateAccess = "true"))
	UTexture2D* EquipIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Dashboard Title", EditCondition = "EquipActorType != EEquipActorType::Default",
			EditConditionHides, AllowPrivateAccess = "true"))
	FString Dashboard_Title = FString(TEXT("DashboardTitle"));
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Dashboard Url", EditCondition = "EquipActorType != EEquipActorType::Default",
			EditConditionHides, AllowPrivateAccess = "true"))
	FGameplayTag Dashboard_Url;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Channel", EditCondition = "EquipActorType == EEquipActorType::MonitorCamera",
			EditConditionHides, AllowPrivateAccess = "true"))
	FString MonitorId = TEXT("0");

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Monitor Name", EditCondition = "EquipActorType == EEquipActorType::MonitorCamera_AI",
			EditConditionHides, AllowPrivateAccess = "true"))
	FString MonitorName = TEXT("");
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Serial No", EditCondition = "EquipActorType == EEquipActorType::MonitorCamera_AI",
			EditConditionHides, AllowPrivateAccess = "true"))
	FString SerialNo = TEXT("0");
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "MonitorChannel", EditCondition = "EquipActorType == EEquipActorType::MonitorCamera_AI",
			EditConditionHides, AllowPrivateAccess = "true"))
	FString MonitorChannel = TEXT("0");

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Received Ai Message", EditCondition =
			"EquipActorType == EEquipActorType::MonitorCamera_AI",
			EditConditionHides, AllowPrivateAccess = "true"))
	bool bIsAiMode = false;


	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Extra Param", EditCondition = "EquipActorType != EEquipActorType::Default",
			EditConditionHides, AllowPrivateAccess = "true"))
	TMap<FString, FString> ExtraParamMap;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EquipSetting",
		meta = (DisplayName = "Web size", EditCondition = "EquipActorType != EEquipActorType::Default",
			EditConditionHides, AllowPrivateAccess = "true"))
	FVector2D MainUISize;
	
	UPROPERTY()
	UDataTable* EquipUrlDataTable;
	static const FString EquipUrlDataTablePath;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ChangeRotAndScale();
	UPROPERTY()
	AAwsaPlayerPawn* PlayerPawn;
	float LastArmLength = 0.f;
	float AdjustedScale = 0.f;
	
	UFUNCTION(BlueprintCallable)
	void ChangeIntroScale();

	UFUNCTION(BlueprintCallable)
	void OnOffMainUI(bool bIsOn);

	UFUNCTION(BlueprintCallable)
	void GetURL(FString& Url) const;

	UFUNCTION(BlueprintCallable)
	FString ConvertTMapToQueryString(const TMap<FString, FString>& InMap) const;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<UAwsaWebUI_ActorDashboard> DefaultIntroWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<UAwsaWebUI_ActorDashboard> DefaultMainWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<UAwsaWebUI_ActorDashboard> DefaultNameWidgetClass;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* IntroUI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* NameUI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* MainUI;

private:
	float OriginScale;
	FVector MouseClickLoc;

#pragma region MonitorRoi
private:
	void UpdateRoiMonitorCameraData();
	UFUNCTION()
	void OnRoiMonitorConfigRequestCompleted(const FJsonStruct& JsonStruct, const FString& WebMsg, const FString& StructName);
public:
	UPROPERTY(BlueprintReadOnly)
	int32 RelatedMonitoring = 0;
	UPROPERTY(BlueprintReadOnly)
	FString ProcedureName = TEXT("");
	UPROPERTY(BlueprintReadOnly)
	FString BeamAssetsName = TEXT("");
	UPROPERTY(BlueprintReadOnly)
	FString BeamLineName = TEXT("");
	
	UFUNCTION(BlueprintImplementableEvent,Category="EquipActor|Widget")
	void SwitchAIIntroWidget();
#pragma endregion

	// Web device data request
private:
	UPROPERTY()
	FDeviceData DeviceData;
public:
	UFUNCTION(BlueprintPure,BlueprintCallable,Category="EquipActor|DeviceData")
	FDeviceData GetDeviceData()
	{
		return DeviceData;
	}
public:
	UFUNCTION()
	void InitDeviceData(FDeviceData TargetDeviceData);
	UFUNCTION(BlueprintImplementableEvent,Category="EquipActor|Widget")
	void InitDevice();

	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EquipSetting")
	float RefArmLength = 2000.f;	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EquipSetting")
	float ClampMaxScale = 20.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EquipSetting")
	float ClampMinScale = 1.f;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="EquipActor|Widget")
	void OnDeviceClicked();

	// TO DO :Local
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "EquipActor|Widget")
	bool bIsLocal = false;
};
