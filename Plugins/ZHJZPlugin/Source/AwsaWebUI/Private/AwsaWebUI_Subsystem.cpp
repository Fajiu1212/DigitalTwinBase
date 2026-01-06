#include "AwsaWebUI_Subsystem.h"

#include "GameplayTagAssetInterface.h"
#include "AwsaWebUI_Interface.h"
#include "AwsaWebUI_Manager.h"
#include "JsonLibraryValue.h"
#include "GenericPlatform/GenericPlatformHttp.h"

// Static ref path
const FString UAwsaWebUI_Subsystem::EnvUrlDataTablePath =
	TEXT("/Script/Engine.DataTable'/Game/Tutorial/AwsaWebUI/DT/DT_EnvUrl.DT_EnvUrl'");
const FString UAwsaWebUI_Subsystem::ConditionDataTablePath =
	TEXT("/Script/Engine.DataTable'/Game/Blueprints/BP_Public/Gameplay/DT_Condition.DT_Condition'");

static const TMap<FString, ETunnelSide> StringToTunnelSideMap = {
	{TEXT("DEFAULT"), ETunnelSide::Default},
	{TEXT("左幅"), ETunnelSide::Left},
	{TEXT("右幅"), ETunnelSide::Right},
	{TEXT("未指定"), ETunnelSide::None}
};

ETunnelSide ParseTunnelSide(const FString& SideString)
{
	if (const ETunnelSide* Found = StringToTunnelSideMap.Find(SideString))
	{
		return *Found;
	}
	return ETunnelSide::None;
}

static const TMap<FString, ETunnelMileageDirection> StringToTunnelMileageDirection = {
	{TEXT("上游方向"), ETunnelMileageDirection::Up},
	{TEXT("下游方向"), ETunnelMileageDirection::Down},
	{TEXT("未指定"), ETunnelMileageDirection::None},
};

ETunnelMileageDirection ParseTunnelMileageDirection(const FString& MileageDirectionString)
{
	if (const ETunnelMileageDirection* Found = StringToTunnelMileageDirection.Find(MileageDirectionString))
	{
		return *Found;
	}
	return ETunnelMileageDirection::None;
}


using namespace UE::Tasks;

UAwsaWebUI_Subsystem::UAwsaWebUI_Subsystem()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> UrlStrFinder(*EnvUrlDataTablePath);
	if (UrlStrFinder.Succeeded())
	{
		EnvUrlDataTable = UrlStrFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> ConditionFinder(*ConditionDataTablePath);
	if (ConditionFinder.Succeeded())
	{
		ConditionDataTable = ConditionFinder.Object;
	}

	CreateWebUIManager();
}

bool UAwsaWebUI_Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void UAwsaWebUI_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FTimerHandle TokenRequestHandle;
	GetWorld()->GetTimerManager().SetTimer(TokenRequestHandle, [this]
	{
		this->TokenRequest();
	}, 0.5f, false, 0.5);
	//TokenRequest();
	DeleteWebCache();

	// UAwsaPipeSubsystem* PipeSubsystem = GetGameInstance()->GetSubsystem<UAwsaPipeSubsystem>();
	// if (PipeSubsystem)
	// {
	// 	FTask TaskA = PipeSubsystem->AddTask([]()
	// 	{
	// 		UAutoParse::PrintLog_GameThread(TEXT("TaskEvent:::WebSubsystem::TaskA"));
	// 		FPlatformProcess::Sleep(5.0f);
	// 	});
	// 	FTask TaskAA = PipeSubsystem->AddTask([]()
	// 	{
	// 		UAutoParse::PrintLog_GameThread(TEXT("TaskEvent:::WebSubsystem::TaskAA"));
	// 		FPlatformProcess::Sleep(5.0f);
	// 	}, TaskA);
	// 	FTask TaskB = PipeSubsystem->AddTask([]()
	// 	{
	// 		UAutoParse::PrintLog_GameThread(TEXT("TaskEvent:::WebSubsystem::TaskB"));
	// 		FPlatformProcess::Sleep(2.0f);
	// 	});
	// 	FTask TaskC = PipeSubsystem->AddTask([]()
	// 	{
	// 		UAutoParse::PrintLog_GameThread(TEXT("TaskEvent:::WebSubsystem::TaskC"));
	// 		FPlatformProcess::Sleep(2.0f);
	// 	}, TArray<FTask>{TaskA, TaskAA, TaskB});
	// }
}

void UAwsaWebUI_Subsystem::Deinitialize()
{
	WebUIManager = nullptr;
	Super::Deinitialize();
}

void UAwsaWebUI_Subsystem::CallWebFunc(const FString& Function, const FJsonLibraryValue& Data)
{
	// for (const auto& Delegate : OnCallWebFunc.GetAllObjectsEvenIfUnreachable())
	// {
	// 	if (Delegate)
	// 	{
	// 		UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("OnCallWebFunc 绑定对象: %s"), *Delegate->GetName()));
	// 	}
	// }
	if (OnCallWebFunc.IsBound())
	{
		OnCallWebFunc.Broadcast(Function, Data);
	}
}

void UAwsaWebUI_Subsystem::CallDiableWebUI(bool bDisable)
{
	if (OnCallDisableWebUI.IsBound())
	{
		OnCallDisableWebUI.Broadcast(bDisable);
	}
}

void UAwsaWebUI_Subsystem::TokenRequest()
{
	FString TokenUrL = GetEnvUrl(
		FGameplayTag::RequestGameplayTag(CheckCondition(TEXT("bIsTestEnv"))
											 ? TEXT("Url.Client.GetToken.Test")
											 : TEXT("Url.Client.GetToken")));
	// FString TokenUrL = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.GetToken")));
	FString TokenVerb = TEXT("POST");
	FString TokenOrgId = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));

	FString TokenContent = FString::Printf(
		TEXT("{\"orgId\" : \"%s\",  \"password\" : \"fb7d5e8e7bbf2154fd88bcfe97c5b474\",  \"userName\" : \"awsa\"}"),
		*TokenOrgId);

	UAutoParse* Instance = UAutoParse::AutoHttpRequest(TokenUrL, TokenContent, TokenVerb,
	                                                   TMap<FString, FString>(), FString(), ERequestType::HttpOnly);
	Instance->OnSuccess.AddDynamic(this, &UAwsaWebUI_Subsystem::SaveTokenRequest);
}

void UAwsaWebUI_Subsystem::SaveTokenRequest(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                            const FString& StructName)
{
	FAutoParseJsonObject JsonMsg;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, JsonMsg);
	FString StrToken = FString();
	UAutoParse::GetStrValueFormObject(TEXT("msg"), JsonMsg, StrToken);
	Token = StrToken;
}

bool UAwsaWebUI_Subsystem::CheckTestEnv()
{
	if (!ConditionDataTable)
	{
		return false;
	}

	FConditionTableRow* Row = ConditionDataTable->FindRow<FConditionTableRow>(TEXT("bIsTestEnv"),TEXT("CheckTestEnv"));

	if (Row && Row->Condition == TEXT("bIsTestEnv"))
	{
		return Row->bCondition;
	}
	return false;
}

bool UAwsaWebUI_Subsystem::CheckCondition(const FString& Condition)
{
	if (!ConditionDataTable || Condition.IsEmpty())
	{
		return false;
	}

	TArray<FConditionTableRow*> ConditionRows;
	ConditionDataTable->GetAllRows(TEXT("CheckCondition"), ConditionRows);

	for (auto RowPtr : ConditionRows)
	{
		if (!RowPtr)
		{
			continue;
		}
		if (RowPtr->Condition.ToLower() == Condition.ToLower())
		{
			return RowPtr->bCondition;
		}
	}
	return false;
}

FString UAwsaWebUI_Subsystem::GetEnvUrl(FGameplayTag UrlTag)
{
	if (!EnvUrlDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnvUrlDataTable is null!"));
		return TEXT("");
	}


	FEnvUrl* Row = EnvUrlDataTable->FindRow<FEnvUrl>(UrlTag.GetTagName(), TEXT("GetEnvUrl"));
	if (Row && Row->UrlTag.IsValid())
	{
		return Row->UrlString;
	}

	UE_LOG(LogTemp, Error, TEXT("UrlTag not found: %s"), *UrlTag.ToString());
	return TEXT("");
}

void UAwsaWebUI_Subsystem::UpdateVersion(FString NewVersion)
{
	FEnvUrl NewRow;
	NewRow.UrlTag = FGameplayTag::RequestGameplayTag(TEXT("Param.Version"));
	NewRow.UrlString = NewVersion;
	EnvUrlDataTable->AddRow(TEXT("Param.Version"), NewRow);
	EnvUrlDataTable->GetOutermost()->MarkPackageDirty();
}

void UAwsaWebUI_Subsystem::CreateWebUIManager()
{
	ConstructorHelpers::FClassFinder<UAwsaWebUI_Manager> BP_WebUIManagerClassFinder(
		TEXT("/Script/Engine.Blueprint'/Game/Tutorial/AwsaWebUI/BP_AwsaWebUIManager.BP_AwsaWebUIManager_C'"));

	if (BP_WebUIManagerClassFinder.Succeeded() && BP_WebUIManagerClassFinder.Class)
	{
		WebUIManager = NewObject<UAwsaWebUI_Manager>(this, BP_WebUIManagerClassFinder.Class, TEXT("BP_WebUIManager"));
		if (WebUIManager)
		{
			WebUIManager->InitManager();
		}
	}
	else
	{
		WebUIManager = NewObject<UAwsaWebUI_Manager>(this, TEXT("WebUIManager"));
		if (WebUIManager)
		{
			WebUIManager->InitManager();
		}
	}
}

void UAwsaWebUI_Subsystem::RegisterListener(UObject* Listener)
{
	if (Listener && Listener->Implements<UAwsaWebUI_Interface>())
	{
		Listeners.AddUnique(Listener);
	}
}

void UAwsaWebUI_Subsystem::UnregisterListener(UObject* Listener)
{
	if (Listener && Listeners.Contains(Listener))
	{
		Listeners.Remove(Listener);
	}
}

void UAwsaWebUI_Subsystem::BroadcastManager(const FString& Name, const FString& Message)
{
	if (WebUIManager)
	{
		IAwsaWebUI_Interface::Execute_ReceiveFunction(WebUIManager.Get(), Name, Message);
	}
}

void UAwsaWebUI_Subsystem::BroadcastMessage(const FString& Name, const FString& Message,
                                            const FGameplayTag& RequiredTag)
{
	for (auto& Listener : Listeners)
	{
		if (Listener.IsValid())
		{
			if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Listener.Get()))
			{
				FGameplayTagContainer ListenerTags;
				TagInterface->GetOwnedGameplayTags(ListenerTags);
				if (ListenerTags.HasTag(RequiredTag))
				{
					IAwsaWebUI_Interface::Execute_ReceiveFunction(Listener.Get(), Name, Message);
				}
			}
		}
	}
}

void UAwsaWebUI_Subsystem::OnHotLoadSuccess(const FString& OldPakFile)
{
	FString OldPakPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / OldPakFile);
	FString ScriptPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Paks/Restart.bat"));
	FString AppPath = FPlatformProcess::ExecutablePath();
	FString AppName = FPaths::GetCleanFilename(AppPath);
	FString ScriptContent;

	ScriptContent = FString::Printf(TEXT(
		"@echo off\n"
		"chcp 65001 > nul\n"

		"taskkill /F /IM \"%s\" > nul 2>&1\r\n"
		"timeout /t 2 /nobreak >nul\n"

		"tasklist | findstr /I \"TracerWebHelper.exe\" > nul\r\n"
		"if %%errorlevel%% equ 0 (\r\n"
		"   taskkill /F /IM TracerWebHelper.exe > nul 2>&1\r\n"
		"   timeout /t 2 /nobreak >nul\r\n"
		")\r\n"
		"timeout /t 2 /nobreak >nul\n"

		"set \"PAK_FILE=%%~dp0%s\"\r\n"
		"if exist \"%%PAK_FILE%%\" del /F /Q \"%%PAK_FILE%%\"\r\n"
		"timeout /t 2 /nobreak >nul\n"

		"set \"ROOT_DIR=%%~dp0..\\..\\..\\\"\r\n"
		"set \"EXE_PATH=%%ROOT_DIR%%%s\"\r\n"
		"start \"\" \"%%EXE_PATH%%\"\r\n"

		"del \"%%~f0\""
	),
	                                *AppName, *OldPakFile.Replace(TEXT("/"), TEXT("\\")),
	                                *FString::Printf(TEXT("%s.exe"), FApp::GetProjectName()));
	if (FFileHelper::SaveStringToFile(ScriptContent, *ScriptPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		FPlatformProcess::CreateProc(*ScriptPath, nullptr, false, true, false, nullptr, 0, nullptr, nullptr);
	}
	//FPlatformProcess::CreateProc(*ScriptPath, nullptr, false, true, false, nullptr, 0, nullptr, nullptr, nullptr, FPlatformProcess::ECreateFlags::CREATE_NO_WINDOW);
}

void UAwsaWebUI_Subsystem::CallOnTestEnvHotLoad()
{
	if (OnTestEnvHotLoad.IsBound())
	{
		OnTestEnvHotLoad.Broadcast();
	}
}

void UAwsaWebUI_Subsystem::OpenCenterPopupWindow(const FString& InComponentStr, const FString& InParamsObjStr)
{
	FAutoParseJsonObject JsonObj;
	JsonObj.AutoParseJsonObject->SetStringField(TEXT("component"), InComponentStr);
	FAutoParseJsonObject ParamsObj;
	UAutoParse::GetJsonObjectFromJsonStr(InParamsObjStr, ParamsObj);
	JsonObj.AutoParseJsonObject->SetObjectField(TEXT("params"), ParamsObj.AutoParseJsonObject);

	UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!WebUI_Subsystem)
	{
		return;
	}
	FJsonLibraryValue WebValue = FString(TEXT("false"));
	WebUI_Subsystem->OnCallWebFunc.Broadcast(TEXT("setBothSides"), WebValue);
}

void UAwsaWebUI_Subsystem::DeleteWebCache()
{
	FString ProjectName = FApp::GetProjectName();
	FString UserProfile = FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"));
	FString TargetDir = UserProfile / TEXT("AppData/Local") / ProjectName / TEXT("Saved/webcache_6367");
	FString FullTargetDir = FPaths::ConvertRelativePathToFull(TargetDir);
	bool bSuccess = IFileManager::Get().DeleteDirectory(*FullTargetDir, false, true);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully deleted: %s"), *FullTargetDir);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to delete: %s"), *FullTargetDir);
	}

	// #if PLATFORM_WINDOWS
	// 	int32 ReturnCode = 0;
	// 	FString StdOut;
	// 	FString StdErr;
	// 	FString Cmd = TEXT("taskkill");
	// 	FString Params = TEXT("/F /IM TracerWebHelper.exe /T");
	//
	// 	FPlatformProcess::ExecProcess(
	// 		*Cmd,                
	// 		*Params,             
	// 		&ReturnCode,         
	// 		&StdOut,             
	// 		&StdErr,             
	// 		nullptr,             
	// 		false                
	// 	);
	//
	// 	if (ReturnCode == 0)
	// 	{
	// 		UE_LOG(LogTemp, Log, TEXT("TracerWebHelper.exe killed successfully. Output: %s"), *StdOut);
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("Failed to kill TracerWebHelper.exe. Error: %s"), *StdErr);
	// 	}
	// #endif
}

void UAwsaWebUI_Subsystem::CallOnNativeSelectionChanged(int32 Index)
{
	if (OnNativeSelectionChanged.IsBound())
	{
		OnNativeSelectionChanged.Broadcast(Index);
	}
}

void UAwsaWebUI_Subsystem::CallModuleSwitched(const FString& InModuleName, const FString& InExtraParam)
{
	ModuleSwitched.Broadcast(InModuleName, InExtraParam);

	if (UWorld* World = GetWorld())
	{
		FTimerHandle DelayHandle;
		World->GetTimerManager().SetTimer(DelayHandle, [this]()
		{
			CallDiableWebUI(false);
		}, 1.f, false);
	}
}

void UAwsaWebUI_Subsystem::CallOnTogglePoiActor(FString TargetType, bool bShow)
{
	if (OnTogglePoiActor.IsBound())
	{
		OnTogglePoiActor.Broadcast(TargetType, bShow);
	}
}

void UAwsaWebUI_Subsystem::RequestListProgress(const FString& TargetModuleName)
{
	FString HttpEnv = GetEnvUrl(
		FGameplayTag::RequestGameplayTag(CheckCondition(TEXT("bIsTestEnv"))
			                                 ? TEXT("Env.Test.GSIot")
			                                 : TEXT("Env.Official.GSIot")));
	FString HttpUrl = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.TunnelProgress")));
	FString AppendOrgId = TEXT("?orgId=") + GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	FString HttpContent = TEXT("");
	FString HttpVerb = TEXT("GET");
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("Authorization"), GetToken());
	FString StructName = TargetModuleName;
	UAutoParse* AutoParseInstance = UAutoParse::AutoHttpRequest(HttpEnv + HttpUrl + AppendOrgId, HttpContent, HttpVerb,
	                                                            Headers,
	                                                            StructName,
	                                                            ERequestType::HttpOnly);
	AutoParseInstance->OnSuccess.AddDynamic(this, &UAwsaWebUI_Subsystem::OnProgressListSuccess);

	// Load local temp
	// FString LocalJsonPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Json/Test.json"));
	// FString WebMsg;
	// if (FFileHelper::LoadFileToString(WebMsg, *LocalJsonPath))
	// {
	// 	OnProgressListSuccess(FJsonStruct(), WebMsg, TEXT(""));
	// }
}

void UAwsaWebUI_Subsystem::OnProgressListSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                 const FString& StructName)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);

	TArray<FAutoParseJsonObject> ParamsObj;
	UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, ParamsObj);

	TunnelWorkFaceSectionArr.Empty();
	TunnelMileageProgressArr.Empty();

	for (const auto& Param : ParamsObj)
	{
		FString TunnelName = TEXT("");
		UAutoParse::GetStrValueFormObject(TEXT("tunnelName"), Param, TunnelName);
		FAutoParseJsonObject DetailListObj;
		UAutoParse::GetObjValueFormObject(TEXT("detailList"), Param, DetailListObj);
		for (const auto& LayerPair : DetailListObj.AutoParseJsonObject->Values)
		{
			FString LayerName = LayerPair.Key;
			int32 LayerIndex = LayerName.IsNumeric() ? FCString::Atoi(*LayerName) : -1;
			for (const auto& SidePair : LayerPair.Value->AsObject()->Values)
			{
				FString SideName = SidePair.Key;
				// Create FTunnelWorkFaceSection
				FTunnelWorkFaceSection NewSection = FTunnelWorkFaceSection(
					TunnelName, LayerIndex, ParseTunnelSide(SideName));
				for (const auto& WorkFaceObj : SidePair.Value->AsArray())
				{
					FAutoParseJsonObject WorkFaceAutoParseJsonObj;
					WorkFaceAutoParseJsonObj.AutoParseJsonObject = WorkFaceObj->AsObject();

					float TotalCompleted = 0.f;
					float TodayMileage = 0.f;
					if (UAutoParse::GetFloatValueFormObject(
						TEXT("totalCompleted"), WorkFaceAutoParseJsonObj, TotalCompleted))
					{
						FString TunnelDirection = TEXT("");
						FString WorkFaceName = TEXT("");
						UAutoParse::GetStrValueFormObject(
							TEXT("tunnelDirection"), WorkFaceAutoParseJsonObj, TunnelDirection);
						UAutoParse::GetStrValueFormObject(TEXT("workFace"), WorkFaceAutoParseJsonObj, WorkFaceName);
						UAutoParse::GetFloatValueFormObject(TEXT("todayMileage"), WorkFaceAutoParseJsonObj, TodayMileage);
						// Create TunnelMileageProgress
						FTunnelMileageProgress TunnelMileageProgress = FTunnelMileageProgress(
							TunnelName, LayerIndex, ParseTunnelSide(SideName),
							ParseTunnelMileageDirection(TunnelDirection), WorkFaceName, TotalCompleted, TodayMileage);
						// Add TunnelMileageProgress
						TunnelMileageProgressArr.Emplace(TunnelMileageProgress);
					}
					else
					{
						FString WorkFaceName = TEXT("");
						float Mileage = 0.0f;
						UAutoParse::GetStrValueFormObject(TEXT("workFace"), WorkFaceAutoParseJsonObj, WorkFaceName);
						UAutoParse::GetFloatValueFormObject(TEXT("workFaceMileage"), WorkFaceAutoParseJsonObj, Mileage);

						// Create TunnelWorkFaceInfo
						FTunnelWorkFaceInfo TunnelWorkFaceInfo = FTunnelWorkFaceInfo(
							TunnelName, LayerIndex, ParseTunnelSide(SideName), WorkFaceName, Mileage);
						// Add workface
						NewSection.AddWorkFace(TunnelWorkFaceInfo);
					}
				}
				// Add FTunnelWorkFaceSection
				TunnelWorkFaceSectionArr.Emplace(NewSection);
			}
		}
	}
	if (OnProgressLoadSuccess.IsBound())
	{
		OnProgressLoadSuccess.Broadcast(StructName);
	}
}

void UAwsaWebUI_Subsystem::UpdateProgressList()
{
}

FTunnelWorkFaceSection* UAwsaWebUI_Subsystem::FindSectionPtr(const FString& TargetTunnelName, int32 TargetLayer,
                                                             ETunnelSide TargetSide)
{
	return TunnelWorkFaceSectionArr.FindByPredicate([&](FTunnelWorkFaceSection& TargetSection)
	{
		return TargetSection.TunnelName.Equals(TargetTunnelName, ESearchCase::IgnoreCase) &&
			TargetSection.Layer == TargetLayer &&
			TargetSection.Side == TargetSide;
	});
}

bool UAwsaWebUI_Subsystem::CalculateTunnelModelDistance(const FString& TunnelName, int32 Layer, ETunnelSide Side,
                                                        const FString& BeginWorkFace, const FString& EndWorkFace,
                                                        float& OutDistance)
{
	const FTunnelWorkFaceSection* SectionPtr = FindSectionPtr(TunnelName, Layer, Side);
	if (SectionPtr)
	{
		return SectionPtr->CalculateDistance(BeginWorkFace, EndWorkFace, OutDistance);
	}
	return false;
}

bool UAwsaWebUI_Subsystem::FindMileageProgressPtr(const FString& TargetTunnelName, int32 TargetLayer,
                                                  ETunnelSide TargetSide, ETunnelMileageDirection TargetDirection,
                                                  const FString& TargetWorkFaceName,
                                                  float& OutCompetedMileage, float& OutTodayMileage)
{
	FTunnelMileageProgress* ProgressionPtr = TunnelMileageProgressArr.FindByPredicate(
		[&](const FTunnelMileageProgress& TargetProgression)
		{
			return TargetProgression.TunnelName.Equals(TargetTunnelName, ESearchCase::IgnoreCase) &&
				TargetProgression.Layer == TargetLayer &&
				TargetProgression.Side == TargetSide &&
				TargetProgression.Direction == TargetDirection &&
				TargetProgression.WorkFaceName == TargetWorkFaceName;
		});
	if (ProgressionPtr)
	{
		OutCompetedMileage = ProgressionPtr->CompletedMileage;
		OutTodayMileage = ProgressionPtr->TodayMileage;
		return true;
	}
	return false;
}

void UAwsaWebUI_Subsystem::RequestSlab(const FString& TargetSlabType, const FString& TargetModuleName)
{
	FString HttpEnv = GetEnvUrl(
		FGameplayTag::RequestGameplayTag(CheckCondition(TEXT("bIsTestEnv"))
			                                 ? TEXT("Env.Test.GSIot")
			                                 : TEXT("Env.Official.GSIot")));
	FString HttpUrl = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.Slab")));
	FString AppendOrgId = TEXT("?orgId=") + GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	FString AppendType = TEXT("&type=") + TargetSlabType;
	FString HttpContent = TEXT("");
	FString HttpVerb = TEXT("GET");
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("Authorization"), GetToken());
	FString StructName = TargetSlabType + TargetModuleName;
	UAutoParse* AutoParseInstance = UAutoParse::AutoHttpRequest(HttpEnv + HttpUrl + AppendOrgId + AppendType,
	                                                            HttpContent, HttpVerb,
	                                                            Headers,
	                                                            StructName,
	                                                            ERequestType::HttpOnly);
	AutoParseInstance->OnSuccess.AddDynamic(this, &UAwsaWebUI_Subsystem::OnSlabRequestSuccess);

	// // Load local temp
	// FString LocalJsonPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Json/Test.json"));
	// FString WebMsg;
	// if (FFileHelper::LoadFileToString(WebMsg, *LocalJsonPath))
	// {
	// 	OnTunnelFloorSuccess(FJsonStruct(), WebMsg, TEXT(""));
	// }
}

void UAwsaWebUI_Subsystem::OnSlabRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                const FString& StructName)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);

	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, DataArr);

	TunnelCompletedFloorsArr.Empty();

	for (auto TunnelObj : DataArr)
	{
		FString TunnelName = TEXT("");
		UAutoParse::GetStrValueFormObject(TEXT("tunnelName"), TunnelObj, TunnelName);

		float TunnelStartMileage = 0.f, TunnelEndMileage = 0.f;
		UAutoParse::GetFloatValueFormObject(TEXT("tunnelStartPoint"), TunnelObj, TunnelStartMileage);
		UAutoParse::GetFloatValueFormObject(TEXT("tunnelEndPoint"), TunnelObj, TunnelEndMileage);
		float TotalTunnelLength = TunnelEndMileage - TunnelStartMileage;

		FTunnelCompletedFloors TunnelCompletedFloors = FTunnelCompletedFloors(TunnelName, TotalTunnelLength);

		bool bHasCompletedFloors = false;

		TArray<FAutoParseJsonObject> SectionList;
		UAutoParse::GetObjArrayValueFormObject(TEXT("sectionList"), TunnelObj, SectionList);

		if (SectionList.Num() != 0)
		{
			for (auto SectionObj : SectionList)
			{
				TArray<FAutoParseJsonObject> SlabDataList;
				UAutoParse::GetObjArrayValueFormObject(TEXT("baseSlabDataList"), SectionObj, SlabDataList);

				if (SlabDataList.Num() != 0)
				{
					for (auto BaseSlabData : SlabDataList)
					{
						int32 Status = 0;
						if (UAutoParse::GetInt32ValueFormObject(TEXT("status"), BaseSlabData, Status))
						{
							if (Status == 3)
							{
								float StartMileage = 0.f, EndMileage = 0.f;
								UAutoParse::GetFloatValueFormObject(TEXT("startMileage"), BaseSlabData, StartMileage);
								UAutoParse::GetFloatValueFormObject(TEXT("endMileage"), BaseSlabData, EndMileage);

								FSingleTunnelFloorInfo FloorInfo = FSingleTunnelFloorInfo(
									TunnelName, StartMileage, EndMileage);
								TunnelCompletedFloors.AddCompletedFloor(FloorInfo);
								
								bHasCompletedFloors = true;
							}
						}
					}
				}
			}
		}

		if (bHasCompletedFloors)
		{
			TunnelCompletedFloorsArr.Emplace(TunnelCompletedFloors);
		}
	}

	if (OnSlabLoadSuccess.IsBound())
	{
		int32 Type = 0;
		FString ModuleName = TEXT("");
		if (!StructName.IsEmpty())
		{
			FString TypeStr = StructName.Left(1);
			Type = FCString::Atoi(*TypeStr);
			ModuleName = StructName.Mid(1);
		}

		OnSlabLoadSuccess.Broadcast(Type, ModuleName);
	}
}

bool UAwsaWebUI_Subsystem::FindCompletedFloorsByTunnelName(const FString& TargetTunnelName,
                                                           TArray<FSingleTunnelFloorInfo>& OutCompletedFloors,
                                                           float& OutTotalTunnelLength)
{
	FTunnelCompletedFloors* CompletedFloorsPtr = TunnelCompletedFloorsArr.FindByPredicate(
		[&](const FTunnelCompletedFloors& TargetTunnelCompletedFloors)
		{
			return TargetTunnelCompletedFloors.TunnelName.Equals(TargetTunnelName);
		});
	if (CompletedFloorsPtr)
	{
		OutCompletedFloors = CompletedFloorsPtr->CompletedFloorInfos;
		OutTotalTunnelLength = CompletedFloorsPtr->TotalTunnelLength;
		return true;
	}
	return false;
}

void UAwsaWebUI_Subsystem::RequestGeologicalForecast()
{
	FString HttpEnv = GetEnvUrl(
		FGameplayTag::RequestGameplayTag(CheckCondition(TEXT("bIsTestEnv"))
			                                 ? TEXT("Env.Test.GSIot")
			                                 : TEXT("Env.Official.GSIot")));
	FString HttpUrl = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.GeologicalForecast")));
	FString AppendOrgId = TEXT("?orgId=") + GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	FString AppendPageSize = TEXT("&pageSize=9999&page=1");
	FString HttpContent = TEXT("");
	FString HttpVerb = TEXT("GET");
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("Authorization"), GetToken());
	FString StructName = TEXT("");
	UAutoParse* AutoParseInstance = UAutoParse::AutoHttpRequest(HttpEnv + HttpUrl + AppendOrgId + AppendPageSize, HttpContent, HttpVerb,
	                                                            Headers,
	                                                            StructName,
	                                                            ERequestType::HttpOnly);
	AutoParseInstance->OnSuccess.AddDynamic(this, &UAwsaWebUI_Subsystem::OnGeologicalForecastRequestSuccess);
}

void UAwsaWebUI_Subsystem::OnGeologicalForecastRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                              const FString& StructName)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);

	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, DataArr);

	GeologicalForecastInfos.Empty();

	for (auto SingleGeologicalForecastObj : DataArr)
	{
		FString TunnelName = TEXT("");
		UAutoParse::GetStrValueFormObject(TEXT("tunnelType"), SingleGeologicalForecastObj, TunnelName);

		FGeologicalForecastInfo& TargetGeologicalForecastInfo =
			GeologicalForecastInfos.FindOrAdd(TunnelName, FGeologicalForecastInfo(TunnelName));


		FString StartMileageStr = TEXT(""), EndMileageStr = TEXT("");
		int32 Level = 0;
		int32 ID = 0;
		UAutoParse::GetStrValueFormObject(TEXT("beginDetectionMileage"), SingleGeologicalForecastObj, StartMileageStr);
		UAutoParse::GetStrValueFormObject(TEXT("endDetectionMileage"), SingleGeologicalForecastObj, EndMileageStr);
		UAutoParse::GetInt32ValueFormObject(TEXT("warningLevel"), SingleGeologicalForecastObj, Level);
		UAutoParse::GetInt32ValueFormObject(TEXT("id"), SingleGeologicalForecastObj, ID);
		int32 StartMileage = StartMileageStr.IsNumeric() ? FCString::Atoi(*StartMileageStr) : 0;
		int32 EndMileage = EndMileageStr.IsNumeric() ? FCString::Atoi(*EndMileageStr) : 0;

		FSingleGeologicalForecast NewSingleGeologicalForecast = FSingleGeologicalForecast(
			TunnelName, StartMileage, EndMileage, Level, ID);
		TargetGeologicalForecastInfo.AddSingleGeologicalForecast(NewSingleGeologicalForecast);
	}
	if (OnGeologicalForecastLoadSuccess.IsBound())
	{
		OnGeologicalForecastLoadSuccess.Broadcast();
	}
}

bool UAwsaWebUI_Subsystem::FindGeologicalForecastByTunnelName(const FString& TunnelName,
                                                              FGeologicalForecastInfo& OutSingleGeologicalForecasts)
{
	if (FGeologicalForecastInfo* FSingleGeologicalForecast = GeologicalForecastInfos.Find(TunnelName))
	{
		OutSingleGeologicalForecasts = *FSingleGeologicalForecast;
		return true;
	}
	return false;
}

bool UAwsaWebUI_Subsystem::FindSlidesArrByTunnelName(const FString& TargetTunnelName,
                                                     TArray<FSingleOverExcavationInfo>& TargetSlidesArr)
{
	FTunnelOverExcavationInfo* TunnelOverExcavationInfo = TunnelOverExcavationInfoArr.FindByPredicate(
		[&](const FTunnelOverExcavationInfo& TargetTunnelOverExcavationInfo)
		{
			return TargetTunnelOverExcavationInfo.TunnelName.Equals(TargetTunnelName);
		});
	if (TunnelOverExcavationInfo)
	{
		TargetSlidesArr = TunnelOverExcavationInfo->SingleOverExcavationInfoArr;
		return true;
	}
	return false;
}

void UAwsaWebUI_Subsystem::RequestTunnelOverExcavation()
{
	FString HttpEnv = GetEnvUrl(
		FGameplayTag::RequestGameplayTag(CheckCondition(TEXT("bIsTestEnv"))
			                                 ? TEXT("Env.Test.GSIot")
			                                 : TEXT("Env.Official.GSIot")));
	FString HttpUrl = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.TunnelExcavation")));
	FString AppendOrgId = TEXT("?orgId=") + GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	FString HttpContent = TEXT("");
	FString HttpVerb = TEXT("GET");
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("Authorization"), GetToken());
	FString StructName = TEXT("");
	UAutoParse* AutoParseInstance = UAutoParse::AutoHttpRequest(
		HttpEnv + HttpUrl + AppendOrgId + TEXT("&status=1&page=1&pageSize=500"), HttpContent, HttpVerb,
		Headers,
		StructName,
		ERequestType::HttpOnly);
	AutoParseInstance->OnSuccess.AddDynamic(this, &UAwsaWebUI_Subsystem::OnTunnelOverExcavationRequestSuccess);
}

void UAwsaWebUI_Subsystem::OnTunnelOverExcavationRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                                const FString& StructName)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);

	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, DataArr);

	TunnelOverExcavationInfoArr.Empty();

	for (auto SingleOverExcavationObj : DataArr)
	{
		FString TunnelName = TEXT("");
		FString MileageStr = TEXT("");
		float Mileage = 0.0f;
		UAutoParse::GetStrValueFormObject(TEXT("tunnelType"), SingleOverExcavationObj, TunnelName);
		UAutoParse::GetStrValueFormObject(TEXT("pileNumber"), SingleOverExcavationObj, MileageStr);
		Mileage = MileageStr.IsNumeric() ? FCString::Atof(*MileageStr) : 0;

		FSingleOverExcavationInfo NewSlideInfo = FSingleOverExcavationInfo(TunnelName, MileageStr, Mileage, false);
		FTunnelOverExcavationInfo* Existing = TunnelOverExcavationInfoArr.FindByPredicate(
			[&](const FTunnelOverExcavationInfo& Item)
			{
				return Item.TunnelName.Equals(TunnelName, ESearchCase::IgnoreCase);
			});
		if (Existing)
		{
			Existing->SingleOverExcavationInfoArr.Add(NewSlideInfo);
		}
		else
		{
			FTunnelOverExcavationInfo NewTunnelInfo = FTunnelOverExcavationInfo(TunnelName);
			NewTunnelInfo.SingleOverExcavationInfoArr.Add(NewSlideInfo);
			TunnelOverExcavationInfoArr.Add(MoveTemp(NewTunnelInfo));
		}
	}

	if (OnOverExcavationLoadSuccess.IsBound())
	{
		OnOverExcavationLoadSuccess.Broadcast();
	}
}

void UAwsaWebUI_Subsystem::CallOnSectionCuttingSelected(const FString& TargetTunnelName)
{
	if (OnSectionCuttingSelected.IsBound())
	{
		OnSectionCuttingSelected.Broadcast(TargetTunnelName);
	}
}

void UAwsaWebUI_Subsystem::CallOnExplosionAnimation(const FString& TargetOwnerTunnelName,bool bResetCamera)
{
	bool bMultiple = CheckCondition(FString::Printf(TEXT("bIsMultipleTunnel_%s"), *TargetOwnerTunnelName));
	if (OnExplosionAnimation.IsBound())
	{
		OnExplosionAnimation.Broadcast(TargetOwnerTunnelName, bResetCamera);
	}
	if (UWorld* World = GetWorld())
	{
		FTimerHandle DelayHandle;
		World->GetTimerManager().SetTimer(DelayHandle, [this]()
		{
			CallWebFunc(TEXT("OnLabelClickComplete"), FJsonLibraryValue());
		}, 2.f, false);
	}
}

bool UAwsaWebUI_Subsystem::FindSurfaceArrByTunnelName(const FString& TargetTunnelName,
                                                      TArray<FSingleSurface>& OutSingleSurfaceArr)
{
	FSurfaceInfo* SurfaceInfo = TunnelSurfaceInfoArr.FindByPredicate(
		[&](const FSurfaceInfo& TargetSurfaceInfo)
		{
			return TargetSurfaceInfo.TunnelName.Equals(TargetTunnelName);
		});

	if (SurfaceInfo)
	{
		OutSingleSurfaceArr = SurfaceInfo->SingleSurfaceArr;
		return true;
	}
	return false;
}

void UAwsaWebUI_Subsystem::RequestTunnelSurfaceInfo()
{
	FString HttpEnv = GetEnvUrl(
		FGameplayTag::RequestGameplayTag(CheckCondition(TEXT("bIsTestEnv"))
			                                 ? TEXT("Env.Test.GSIot")
			                                 : TEXT("Env.Official.GSIot")));
	FString HttpUrl = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.TunnelSurface")));
	FString AppendOrgId = TEXT("/") + GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	FString HttpContent = TEXT("");
	FString HttpVerb = TEXT("GET");
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("Authorization"), GetToken());
	FString StructName = TEXT("");
	UAutoParse* AutoParseInstance = UAutoParse::AutoHttpRequest(HttpEnv + HttpUrl + AppendOrgId, HttpContent, HttpVerb,
	                                                            Headers,
	                                                            StructName,
	                                                            ERequestType::HttpOnly);
	AutoParseInstance->OnSuccess.AddDynamic(this, &UAwsaWebUI_Subsystem::OnTunnelSurfaceInfoRequestSuccess);
}

void UAwsaWebUI_Subsystem::OnTunnelSurfaceInfoRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                             const FString& StructName)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);

	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, DataArr);

	TunnelSurfaceInfoArr.Empty();

	for (auto SingleTunnelSurfaceObj : DataArr)
	{
		FString TunnelName = TEXT("");
		FString PileNumber = TEXT("");
		float Mileage = 0.f;
		int32 StatusInt = 0;
		UAutoParse::GetStrValueFormObject(TEXT("tunnelName"), SingleTunnelSurfaceObj, TunnelName);
		UAutoParse::GetStrValueFormObject(TEXT("pileNumber"), SingleTunnelSurfaceObj, PileNumber);
		UAutoParse::GetFloatValueFormObject(TEXT("mileage"), SingleTunnelSurfaceObj, Mileage);
		UAutoParse::GetInt32ValueFormObject(TEXT("status"), SingleTunnelSurfaceObj, StatusInt);
		bool Status = StatusInt == 1 ? true : false;

		FSingleSurface NewSingleSurface = FSingleSurface(TunnelName, PileNumber, Mileage, Status);
		FSurfaceInfo* Existing = TunnelSurfaceInfoArr.FindByPredicate(
			[&](const FSurfaceInfo& Item)
			{
				return Item.TunnelName.Equals(TunnelName, ESearchCase::IgnoreCase);
			});
		if (Existing)
		{
			Existing->SingleSurfaceArr.Add(NewSingleSurface);
		}
		else
		{
			FSurfaceInfo NewSurface = FSurfaceInfo(TunnelName);
			NewSurface.SingleSurfaceArr.Add(NewSingleSurface);
			TunnelSurfaceInfoArr.Add(MoveTemp(NewSurface));
		}
	}

	if (OnSurfaceInfoLoadSuccess.IsBound())
	{
		OnSurfaceInfoLoadSuccess.Broadcast();
	}
}

bool UAwsaWebUI_Subsystem::FindTunnelVehicleInfoByTunnelName(const FString& TunnelName,
		FTunnelVehicleInfo& OutTunnelVehicleInfo)
{
	FTunnelVehicleInfo* TunnelVehicleInfo = TunnelVehicleInfoArr.FindByPredicate(
		[&](const FTunnelVehicleInfo& TargetTunnelVehicleInfo)
		{
			return TargetTunnelVehicleInfo.TunnelName.Equals(TunnelName, ESearchCase::CaseSensitive);
		});

	if (TunnelVehicleInfo)
	{
		OutTunnelVehicleInfo = *TunnelVehicleInfo;
		return true;
	}
	return false;
}

void UAwsaWebUI_Subsystem::RequestTunnelVehicle()
{
	FString HttpEnv = GetEnvUrl(
		FGameplayTag::RequestGameplayTag(CheckCondition(TEXT("bIsTestEnv"))
											 ? TEXT("Env.Test.GSIot")
											 : TEXT("Env.Official.GSIot")));
	FString HttpUrl = GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.TunnelVehicle")));
	FString AppendOrgId = TEXT("?orgId=") + GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	FString HttpContent = TEXT("");
	FString HttpVerb = TEXT("GET");
	TMap<FString, FString> Headers;
	Headers.Add(TEXT("Authorization"), GetToken());
	FString StructName = TEXT("");
	UAutoParse* AutoParseInstance = UAutoParse::AutoHttpRequest(HttpEnv + HttpUrl + AppendOrgId, HttpContent, HttpVerb,
																Headers,
																StructName,
																ERequestType::HttpOnly);
	if (AutoParseInstance)
	{
		AutoParseInstance->OnSuccess.AddDynamic(this, &UAwsaWebUI_Subsystem::OnTunnelVehicleRequestSuccess);
	}
}

void UAwsaWebUI_Subsystem::OnTunnelVehicleRequestSuccess(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                         const FString& StructName)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);
	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(TEXT("data"), MsgObj, DataArr);

	UAutoParse::PrintLog_GameThread(FDateTime::Now().ToString());
	for ( const auto& SingleTunnelVehicleObj : DataArr )
	{
		FString TunnelName = TEXT("");
		UAutoParse::GetStrValueFormObject(TEXT("tunnelName"), SingleTunnelVehicleObj, TunnelName);
		
	}
}

void UAwsaWebUI_Subsystem::CallOnMinimapToggleClicked(bool bShowMap)
{
	if (OnMiniMapToggleClicked.IsBound())
	{
		OnMiniMapToggleClicked.Broadcast(bShowMap);
	}
}

void UAwsaWebUI_Subsystem::CallOnMiniMapIconClicked(const FString& TargetLocStr)
{
	if (OnMiniMapIconClicked.IsBound())
	{
		OnMiniMapIconClicked.Broadcast(TargetLocStr);
	}
}

void UAwsaWebUI_Subsystem::CallOnMonitorOverviewPageChanged(int32 TargetPageIndex)
{
	if (OnMonitorOverviewPageChanged.IsBound())
	{
		OnMonitorOverviewPageChanged.Broadcast(TargetPageIndex);
	}
}

void UAwsaWebUI_Subsystem::CallOnMonitorClosed()
{
	if (OnMonitorClosed.IsBound())
	{
		OnMonitorClosed.Broadcast();
	}
}
//
// FString UAwsaWebUI_Subsystem::UrlEncode(const FString& SourceStr)
// {
// 	return FGenericPlatformHttp::UrlEncode(SourceStr);
// }
//
