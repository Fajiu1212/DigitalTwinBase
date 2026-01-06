#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EquipActorSubsystem.generated.h"

struct FJsonStruct;
class ABaseEquipActor;

USTRUCT(BlueprintType)
struct FDeviceData
{
	GENERATED_BODY()
	FDeviceData()
	{
	}

	FDeviceData(int32 InDeviceID, int32 InDeviceExerciseMethod, const FString& InDeviceLocation,
	            const FString& InIconURL,
	            const FString& InDeviceName, const FString& InDeviceType, const FString& InParams)
		: DeviceID(InDeviceID),
		  DeviceExerciseMethod(InDeviceExerciseMethod),
		  IconURL(InIconURL),
		  DeviceName(InDeviceName),
		  DeviceType(InDeviceType),
		  Params(InParams)
	{
		TArray<FString> Parts;
		InDeviceLocation.ParseIntoArray(Parts, TEXT("，"), true);
		if (Parts.Num() == 3)
		{
			float X = FCString::Atof(*Parts[0]);
			float Y = FCString::Atof(*Parts[1]);
			float Z = FCString::Atof(*Parts[2]);
			DeviceLocation = FVector(X, Y, Z);
		}
		else
		{
			DeviceLocation = FVector::ZeroVector;
		}
	}

	// UID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeviceID = -1;
	// Unreal parms
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeviceExerciseMethod = 2; // 1:Dynamic, 2:Static
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector DeviceLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IconURL = TEXT("");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DeviceName = TEXT("");
	// To web params
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DeviceType = TEXT("");
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Params = TEXT("");

	friend uint32 GetTypeHash(const FDeviceData& Arg)
	{
		return GetTypeHash(Arg.DeviceID);
	}

	friend bool operator==(const FDeviceData& Lhs, const FDeviceData& RHS)
	{
		return Lhs.DeviceID == RHS.DeviceID;
	}

	friend bool operator!=(const FDeviceData& Lhs, const FDeviceData& RHS)
	{
		return !(Lhs == RHS);
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlacePreviewMode, bool, bIsPlacePreviewMode);

UCLASS()
class ZHJZPLUGIN_API UEquipActorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UEquipActorSubsystem();
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Update procedure equip actors
	TArray<TSharedPtr<FDeviceData>> SafeDeviceData;
	UFUNCTION(BlueprintCallable, Category="EquipActorSubsystem")
	void RequestDevice();
	UFUNCTION()
	void OnRequestDeviceSucceed(const FJsonStruct& JsonStruct, const FString&
	                            WebMsg, const FString& StructName);
	UFUNCTION()
	void CreateDevice();

	// Device
	UPROPERTY()
	TArray<ABaseEquipActor*> SafeEquipActors;

	UFUNCTION(BlueprintCallable, Category="EquipActorSubsystem")
	void AddSafeEquipActors(ABaseEquipActor* TargetEquipActor);

public:
	UFUNCTION(BlueprintCallable, Category="EquipActorSubsystem")
	void HideSafeEquipActors(bool bHide);
	UPROPERTY(BlueprintAssignable, BlueprintReadWrite)
	FOnPlacePreviewMode OnPlacePreviewMode;
	UFUNCTION(BlueprintCallable, Category="EquipActorSubsystem")
	void CallOnPlacePreviewMode(bool bIsPlacePreviewMode)
	{
		OnPlacePreviewMode.Broadcast(bIsPlacePreviewMode);
	}

	// TO DO: fix special type
	UFUNCTION(BlueprintCallable, Category="EquipActorSubsystem")
	void ShowSpecificDevices(const FString& DeviceName);
	UFUNCTION(BlueprintCallable, Category="EquipActorSubsystem")
	void ShowExceptSpecificDevices(const FString& DeviceName);
	UFUNCTION(BlueprintCallable, Category="EquipActorSubsystem")
	void MoveToDeviceBySpecificDeviceName(const FString& TargetDeviceName, const FString& DeviceParam);

	//
};
