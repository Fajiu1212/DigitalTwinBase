#include "Actor/EquipActorSubsystem.h"

#include "AwsaWebUI_Subsystem.h"
#include "Actor/BaseEquipActor.h"
#include "Gameplay/AwsaPlayerPawn.h"
#include "Kismet/GameplayStatics.h"

UEquipActorSubsystem::UEquipActorSubsystem()
{
}

bool UEquipActorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void UEquipActorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEquipActorSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UEquipActorSubsystem::RequestDevice()
{
	TArray<ABaseEquipActor*> ActorsToRemove;
	for (ABaseEquipActor* Actor : SafeEquipActors)
	{
		if (!Actor->bIsLocal)
		{
			ActorsToRemove.Add(Actor);
		}
	}
	for (ABaseEquipActor* Actor : ActorsToRemove)
	{
		SafeEquipActors.Remove(Actor);
		Actor->Destroy();
	}

	UAwsaWebUI_Subsystem* WebUI_Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!WebUI_Subsystem)
	{
		return;
	}
	bool bIsTestEnv = WebUI_Subsystem->CheckTestEnv();
	FString RequestEnv = WebUI_Subsystem->GetEnvUrl(
		FGameplayTag::RequestGameplayTag(bIsTestEnv ? TEXT("Env.Test.GSIot") : TEXT("Env.Official.GSIot")));
	FString RequestURL = WebUI_Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.RequestDevice")));
	//FString OrgId = WebUI_Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	FString OrgId = WebUI_Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.TestOrgId")));
	FString RequestUrl = FString::Printf(TEXT("%s%s?orgId=%s&page=1&pageSize=99"), *RequestEnv, *RequestURL, *OrgId);
	UAutoParse::PrintLog_GameThread(RequestUrl);
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("Authorization"), WebUI_Subsystem->GetToken());
	UAutoParse* RequestInstance = UAutoParse::AutoHttpRequest(RequestUrl, TEXT(""),TEXT("GET"),
	                                                          Headers, TEXT(""));
	RequestInstance->OnSuccess.AddDynamic(this, &UEquipActorSubsystem::OnRequestDeviceSucceed);
}

void UEquipActorSubsystem::OnRequestDeviceSucceed(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                  const FString& StructName)
{
	UAutoParse::PrintLog_GameThread(WebMsg);

	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);
	TArray<FAutoParseJsonObject> DataArray;
	UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, DataArray);

	if (DataArray.IsEmpty())
	{
		return;
	}
	SafeDeviceData.Empty();

	for (auto& Data : DataArray)
	{
		int32 DeviceID = -1;
		int32 DeviceExerciseMethod = 2;
		FString DeviceLocation = TEXT("");
		FString IconURL = TEXT("");
		FString DeviceName = TEXT("");
		FString DeviceType = TEXT("");
		FString Params = TEXT("");

		if (UAutoParse::GetInt32ValueFormObject(TEXT("id"), Data, DeviceID))
		{
			UAutoParse::GetInt32ValueFormObject(TEXT("ueExerciseMethods"), Data, DeviceExerciseMethod);
			UAutoParse::GetStrValueFormObject(TEXT("deviceLocation"), Data, DeviceLocation);
			UAutoParse::GetStrValueFormObject(TEXT("ueIconLink"), Data, IconURL);
			UAutoParse::GetStrValueFormObject(TEXT("deviceName"), Data, DeviceName);
			UAutoParse::GetStrValueFormObject(TEXT("deviceType"), Data, DeviceType);

			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Params);
			FJsonSerializer::Serialize(Data.AutoParseJsonObject.ToSharedRef(), Writer);
			SafeDeviceData.Add(MakeShared<FDeviceData>(FDeviceData(DeviceID, DeviceExerciseMethod, DeviceLocation,
			                                                       IconURL, DeviceName, DeviceType, Params)));
		}
	}
	CreateDevice();
}

void UEquipActorSubsystem::CreateDevice()
{
	if (SafeDeviceData.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}


	for (const TSharedPtr<FDeviceData>& Params : SafeDeviceData)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		SpawnInfo.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FString BlueprintPath = TEXT("/Game/Blueprints/BP_Public/EquipActor/Equip_BP.Equip_BP_C");
		UClass* EquipBPClass = LoadClass<ABaseEquipActor>(nullptr, *BlueprintPath);
		if (!EquipBPClass)
		{
			return;
		}

		ABaseEquipActor* NewActor = World->SpawnActor<ABaseEquipActor>(
			EquipBPClass,
			Params.Get()->DeviceLocation,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		if (NewActor)
		{
			SafeEquipActors.Add(NewActor);
			NewActor->InitDeviceData(*Params.Get());
			NewActor->InitDevice();
		}
	}
}

void UEquipActorSubsystem::AddSafeEquipActors(ABaseEquipActor* TargetEquipActor)
{
	if (TargetEquipActor)
	{
		SafeEquipActors.Add(TargetEquipActor);
	}
}

void UEquipActorSubsystem::HideSafeEquipActors(bool bHide)
{
	if (SafeEquipActors.IsEmpty())
	{
		return;
	}
	for (const auto& SafeActor : SafeEquipActors)
	{
		if (SafeActor)
		{
			SafeActor->SetActorHiddenInGame(bHide);
			SafeActor->SetActorEnableCollision(!bHide);
		}
	}
}

void UEquipActorSubsystem::ShowSpecificDevices(const FString& DeviceName)
{
	for (const auto& SafeActor : SafeEquipActors)
	{
		if (SafeActor)
		{
			if (SafeActor->GetDeviceData().DeviceType.Equals(DeviceName, ESearchCase::IgnoreCase))
			{
				SafeActor->SetActorHiddenInGame(false);
				SafeActor->SetActorEnableCollision(true);
			}
		}
	}
}

void UEquipActorSubsystem::ShowExceptSpecificDevices(const FString& DeviceName)
{
	for (const auto& SafeActor : SafeEquipActors)
	{
		if (SafeActor)
		{
			if (!SafeActor->GetDeviceData().DeviceType.Equals(DeviceName, ESearchCase::IgnoreCase))
			{
				SafeActor->SetActorHiddenInGame(false);
				SafeActor->SetActorEnableCollision(true);
			}
		}
	}
}

void UEquipActorSubsystem::MoveToDeviceBySpecificDeviceName(const FString& TargetDeviceName, const FString& DeviceParam)
{
	ABaseEquipActor* TargetActor = nullptr;
	for (const auto& SafeActor : SafeEquipActors)
	{
		if (SafeActor)
		{
			if (SafeActor->GetDeviceData().DeviceName.Equals(TargetDeviceName, ESearchCase::IgnoreCase))
			{
				TargetActor = SafeActor;
				break;
			}
		}
	}
	if (TargetActor)
	{
		TargetActor->OnDeviceClicked();
	}
	else
	{
		//TO DO: Open single monitor widget
		if (AAwsaPlayerPawn* PlayerPawn = Cast<AAwsaPlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
		{
			PlayerPawn->MoveToNoDeviceMonitoring(DeviceParam);
		}
	}
}
