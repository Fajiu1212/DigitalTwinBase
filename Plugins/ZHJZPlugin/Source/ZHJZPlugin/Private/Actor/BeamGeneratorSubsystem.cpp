#include "Actor/BeamGeneratorSubsystem.h"

#include "AwsaWebUI_Subsystem.h"
#include "Actor/BeamActorBase.h"
#include "Kismet/GameplayStatics.h"

bool UBeamGeneratorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void UBeamGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UBeamGeneratorSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UBeamGeneratorSubsystem::InitProduction(const FString& WebMsg)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);

	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(FString("data"), MsgObj, DataArr);

	TArray<FAutoParseJsonObject> LocalData = MoveTemp(DataArr);

	AsyncTask(ENamedThreads::Type::AnyBackgroundThreadNormalTask, [
		          WeakThis= TWeakObjectPtr<UBeamGeneratorSubsystem>(this),LocalData]()
	          {
		          if (!WeakThis.IsValid())
		          {
			          return;
		          }

		          TMap<FString, FBeamStruct> BeamMap;
	          });
}

void UBeamGeneratorSubsystem::InitBeamGenerators()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	TArray<AActor*> GeneratorActors;
	UGameplayStatics::GetAllActorsOfClass(World, ABeamActorBase::StaticClass(), GeneratorActors);
	StaticPedestals.Empty();
	for (auto Actor : GeneratorActors)
	{
		ABeamActorBase* BeamActor = Cast<ABeamActorBase>(Actor);
		if (BeamActor && BeamActor->GetBeamType() == EAiType::AiUnsupported)
		{
			StaticPedestals.Add(BeamActor);
		}
		else if (BeamActor && BeamActor->GetBeamType() == EAiType::AiUnsupported == 1)
		{
			AiPedestals.Add(BeamActor);
		}
	}
}

void UBeamGeneratorSubsystem::RequestProduction()
{
	RequestProduction_Ai();
	RequestProduction_Static();
}

void UBeamGeneratorSubsystem::RequestProduction_Ai()
{
	UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!WebUI_Subsystem)
	{
		return;
	}

	const FString RequestEnv = WebUI_Subsystem->CheckCondition(TEXT("bIsTestEnv"))
		                           ? WebUI_Subsystem->GetEnvUrl(
			                           FGameplayTag::RequestGameplayTag(TEXT("Env.Test.Production")))
		                           : WebUI_Subsystem->GetEnvUrl(
			                           FGameplayTag::RequestGameplayTag(TEXT("Env.Official.Production")));

	const FString RequestURL = WebUI_Subsystem->GetEnvUrl(
		FGameplayTag::RequestGameplayTag(TEXT("Url.Client.InitAiProduction")));
	const FString RequestContent = TEXT("");
	const FString RequestVerb = TEXT("GET");
	const FString RequestAppCode = WebUI_Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.AppCode")));
	const FString RequestOrgId = WebUI_Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));

	TMap<FString, FString> RequestHeaders;
	RequestHeaders.Add(TEXT("Authorization"), WebUI_Subsystem->GetToken());


	FString CombineUrl = FString::Printf(
		TEXT("%s%s?appCode=%s&orgId=%s"),
		*RequestEnv,
		*RequestURL,
		*RequestAppCode,
		*RequestOrgId);
	TWeakObjectPtr<UBeamGeneratorSubsystem> WeakThis = this;

	UAutoParse* RequestInstance = UAutoParse::AutoHttpRequest(CombineUrl, RequestContent, RequestVerb,
	                                                          RequestHeaders, TEXT(""));
	RequestInstance->OnSuccess.AddDynamic(WeakThis.Get(), &UBeamGeneratorSubsystem::OnRequestAiProductionComplete);
}

void UBeamGeneratorSubsystem::ProcessingProductionData_Ai(const FString& WebMsg)
{
	NewAiPedestalsRequestMap.Empty();

	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);

	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(FString("data"), MsgObj, DataArr);

	TArray<FAutoParseJsonObject> LocalData = MoveTemp(DataArr);
	AsyncTask(ENamedThreads::Type::AnyBackgroundThreadNormalTask, [
		          WeakThis = TWeakObjectPtr<UBeamGeneratorSubsystem>(this),LocalData]()
	          {
		          if (!WeakThis.IsValid())
		          {
			          return;
		          }
		          TMap<FString, FBeamStruct> TempAiPedestals;
		          TempAiPedestals.Reserve(LocalData.Num());

		          for (auto PedestalObj : LocalData)
		          {
			          // Pedestal
			          // 台座ID（唯一标识符）
			          int32 InBeamAssetsId = -1;
			          //台座名称
			          FString InBeamAssetsName = TEXT("");
			          //区域
			          FString InArea = TEXT("");
			          // 工序名称
			          FString InProcedureName = TEXT("");
			          // 识别图像
			          FString InRoiImage = TEXT("");
			          // 识别时间
			          FString InIdentifyTime = TEXT("");
			          //关联Monitor
			          FString InRelatedMonitoringName = TEXT("");

			          //Beam
			          // 梁id
			          int32 InBeamId = -1;
			          // 梁编号
			          FString InBeamCode = TEXT("");
			          //子生产线
			          FString InBeamLineChildConfigName = TEXT("");
			          // 计划架设时间
			          FString InPlannedBeginDate = TEXT("");
			          // 梁型
			          FString InBeamMoldConfigName = TEXT("");
			          // 标段名称
			          FString InBeamSectionConfigName = TEXT("");
			          // 桥段名称
			          FString InBridgeName = TEXT("");

			          UAutoParse::GetInt32ValueFormObject(FString("beamAssetsId"), PedestalObj, InBeamAssetsId);
			          UAutoParse::GetStrValueFormObject(FString("beamAssetsName"), PedestalObj, InBeamAssetsName);
			          UAutoParse::GetStrValueFormObject(FString("area"), PedestalObj, InArea);
			          UAutoParse::GetStrValueFormObject(FString("procedureName"), PedestalObj, InProcedureName);
			          UAutoParse::GetStrValueFormObject(FString("roiImage"), PedestalObj, InRoiImage);
			          UAutoParse::GetStrValueFormObject(FString("identifyTime"), PedestalObj, InIdentifyTime);
			          UAutoParse::GetStrValueFormObject(FString("relatedMonitoring"), PedestalObj,
			                                            InRelatedMonitoringName);
			          UAutoParse::GetStrValueFormObject(FString(TEXT("beamLineChildName")), PedestalObj,
			                                            InBeamLineChildConfigName);

			          FAutoParseJsonObject BeamRes;
			          if (UAutoParse::GetObjValueFormObject(TEXT("beamRes"), PedestalObj, BeamRes))
			          {
				          UAutoParse::GetInt32ValueFormObject(FString("id"), BeamRes, InBeamId);
				          UAutoParse::GetStrValueFormObject(FString("beamCode"), BeamRes, InBeamCode);
				          UAutoParse::GetStrValueFormObject(FString("beamLineChildConfigName"), BeamRes,
				                                            InBeamLineChildConfigName);
				          UAutoParse::GetStrValueFormObject(FString("plannedBeginDate"), BeamRes,
				                                            InPlannedBeginDate);
				          UAutoParse::GetStrValueFormObject(FString("beamMoldConfigName"), BeamRes,
				                                            InBeamMoldConfigName);
				          UAutoParse::GetStrValueFormObject(FString("beamSectionConfigName"), BeamRes,
				                                            InBeamSectionConfigName);
				          UAutoParse::GetStrValueFormObject(FString("bridgeName"), BeamRes, InBridgeName);
			          }

			          TempAiPedestals.Emplace(InBeamAssetsName,
			                                  FBeamStruct(InBeamAssetsId,
			                                              InBeamAssetsName,
			                                              InArea,
			                                              InProcedureName,
			                                              InRoiImage,
			                                              InIdentifyTime,
			                                              InRelatedMonitoringName,
			                                              InBeamId,
			                                              InBeamCode,
			                                              InBeamLineChildConfigName,
			                                              InPlannedBeginDate,
			                                              InBeamMoldConfigName,
			                                              InBeamSectionConfigName,
			                                              InBridgeName));
		          }
					if (!WeakThis.IsValid()) return;

		          WeakThis->NewAiPedestalsRequestMap = MoveTemp(TempAiPedestals);
		          AsyncTask(ENamedThreads::Type::GameThread, [WeakThis]()
		          {
			          check(WeakThis.IsValid());
			          WeakThis->GenerateProduction_Ai();
		          });
	          });
}

void UBeamGeneratorSubsystem::RequestProduction_Static()
{
	UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!WebUI_Subsystem)
	{
		return;
	}

	const FString RequestEnv = WebUI_Subsystem->CheckCondition(TEXT("bIsTestEnv"))
		                           ? WebUI_Subsystem->GetEnvUrl(
			                           FGameplayTag::RequestGameplayTag(TEXT("Env.Test.Production")))
		                           : WebUI_Subsystem->GetEnvUrl(
			                           FGameplayTag::RequestGameplayTag(TEXT("Env.Official.Production")));
	const FString RequestURL = WebUI_Subsystem->
		GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.InitBeam")));;
	const FString RequestContent = TEXT("");
	const FString RequestVerb = TEXT("Get");

	TMap<FString, bool> StaticQueryMap = {};
	StaticQueryMap.Add(TEXT("1"), WebUI_Subsystem->CheckCondition(TEXT("bRequestGjbz")));
	StaticQueryMap.Add(TEXT("2"), WebUI_Subsystem->CheckCondition(TEXT("bRequestZltz")));
	StaticQueryMap.Add(TEXT("3"), WebUI_Subsystem->CheckCondition(TEXT("bRequestCltz")));

	FString RequestAppCode = WebUI_Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.AppCode")));
	FString RequestOrgId = WebUI_Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));

	TMap<FString, FString> RequestHeader;
	RequestHeader.Add(TEXT("Authorization"), WebUI_Subsystem->GetToken());

	TWeakObjectPtr<UBeamGeneratorSubsystem> WeakThis = this;

	for (const TPair<FString, bool>& Pair : StaticQueryMap)
	{
		if (Pair.Value == true)
		{
			FString RequestType = Pair.Key;
			FString CombineUrl = FString::Printf(
				TEXT("%s%s?appCode=%s&orgId=%s&type=%s"),
				*RequestEnv,
				*RequestURL,
				*RequestAppCode,
				*RequestOrgId,
				*RequestType);

			UAutoParse* InitInstance = UAutoParse::AutoHttpRequest(
				CombineUrl, RequestContent, RequestVerb, RequestHeader, RequestType, ERequestType::Auto);
			InitInstance->OnSuccess.AddDynamic(WeakThis.Get(),
			                                   &UBeamGeneratorSubsystem::OnRequestStaticProductionComplete);
		}
	}
}

void UBeamGeneratorSubsystem::ProcessingProductionData_Gjbz(const FString& WebMsg)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);
	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(FString("data"), MsgObj, DataArr);

	TArray<FAutoParseJsonObject> LocalData = MoveTemp(DataArr);

	AsyncTask(ENamedThreads::Type::AnyBackgroundThreadNormalTask,
	          [WeakThis = TWeakObjectPtr<UBeamGeneratorSubsystem>(this),LocalData]()
	          {
		          if (!WeakThis.IsValid())
		          {
			          return;
		          }

		          TMap<FString, FBeamStruct> TempStaticPedestals_Gjbz;
		          TempStaticPedestals_Gjbz.Reserve(LocalData.Num());

		          for (auto PedestalObj : LocalData)
		          {
			          FString InName = TEXT(""); // 台座名称
			          int32 InAssetsId = -1; // 台座id

			          UAutoParse::GetInt32ValueFormObject(FString("assetsId"), PedestalObj, InAssetsId);

			          if (InAssetsId != -1)
			          {
				          UAutoParse::GetStrValueFormObject(FString("name"), PedestalObj, InName);
				          TempStaticPedestals_Gjbz.Emplace(InName, FBeamStruct(
					                                           InAssetsId, InName));
			          }
		          }
		          check(WeakThis.IsValid());
		          WeakThis->StaticPedestalsRequestMap_Gjbz = MoveTemp(TempStaticPedestals_Gjbz);
		          AsyncTask(ENamedThreads::GameThread, [WeakThis]()
		          {
			          if (WeakThis.IsValid())
			          {
				          WeakThis->GenerateProduction_Gjbz();
			          }
		          });
	          });
}

void UBeamGeneratorSubsystem::ProcessingProductionData_Zltz(const FString& WebMsg)
{
	FAutoParseJsonObject MsgObj;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, MsgObj);
	TArray<FAutoParseJsonObject> DataArr;
	UAutoParse::GetObjArrayValueFormObject(FString("data"), MsgObj, DataArr);

	TArray<FAutoParseJsonObject> LocalData = MoveTemp(DataArr);

	AsyncTask(ENamedThreads::Type::AnyBackgroundThreadNormalTask,
	          [WeakThis = TWeakObjectPtr<UBeamGeneratorSubsystem>(this),LocalData]()
	          {
		          if (!WeakThis.IsValid())
		          {
			          return;
		          }
		          // Loop pedestals
		          TMap<FString, FBeamStruct> TempPedestals_Single;
		          TempPedestals_Single.Reserve(LocalData.Num());

		          for (auto PedestalObj : LocalData)
		          {
			          int32 InBeamId = -1; // 梁id（唯一标识符）
			          UAutoParse::GetInt32ValueFormObject(FString("beamId"), PedestalObj, InBeamId);
			          if (InBeamId != -1)
			          {
				          // int32 InBeamFieldConfigId = -1; // 梁场配置id
				          // int32 InBeamLineChildConfigId = -1; // 子生产线
				          // int32 InBeamStatusINT32 = -1; // 梁状态
				          FString InBeamMoldConfigName = TEXT(""); // 梁型名称
				          // int32 bIsAiIdentify = 2;
				          FString InBeamCode = TEXT(""); // 梁编号
				          FString InBeamBridgeConfigName = TEXT(""); // 桥段名称beamLineConfigId
				          FString InBeamSectionConfigName = TEXT("");
				          FString InProcedureName = TEXT(""); // 工序名称
				          FString InName = TEXT(""); // 台座名称
				          // int32 InBeamLineConfigId = -1; // 生产线配置id
				          FString InBeamLineChildConfigName = FString(); // 子生产线名称
				          //int32 InBeamId = -1; // 梁id（唯一标识符）
				          int32 AssetsId = -1;

				          UAutoParse::GetStrValueFormObject(FString("beamMoldConfigName"), PedestalObj,
				                                            InBeamMoldConfigName);
				          UAutoParse::GetStrValueFormObject(FString("beamCode"), PedestalObj, InBeamCode);
				          UAutoParse::GetStrValueFormObject(FString("beamBridgeConfigName"), PedestalObj,
				                                            InBeamBridgeConfigName);
				          UAutoParse::GetStrValueFormObject(FString("beamSectionConfigName"), PedestalObj,
				                                            InBeamSectionConfigName);
				          UAutoParse::GetStrValueFormObject(FString("procedureName"), PedestalObj, InProcedureName);
				          UAutoParse::GetStrValueFormObject(FString("name"), PedestalObj, InName);
				          UAutoParse::GetStrValueFormObject(FString("beamLineChildConfigName"), PedestalObj,
				                                            InBeamLineChildConfigName);


				          UAutoParse::GetInt32ValueFormObject(FString("assetsId"), PedestalObj, AssetsId);
				          TempPedestals_Single.Emplace(InName, FBeamStruct(InBeamMoldConfigName, InBeamCode,
				                                                           InBeamBridgeConfigName,
				                                                           InBeamSectionConfigName, InProcedureName,
				                                                           InName, InBeamLineChildConfigName,
				                                                           AssetsId));
			          }
		          	
			          check(WeakThis.IsValid());
			          WeakThis->StaticPedestalsRequestMap_Zltz = MoveTemp(TempPedestals_Single);
			          AsyncTask(ENamedThreads::GameThread, [WeakThis]()
			          {
				          if (WeakThis.IsValid())
				          {
					          WeakThis->GenerateProduction_Zltz();
				          }
			          });
		          }
	          });
}

void UBeamGeneratorSubsystem::ProcessingProductionData_Cltz(const FString& WebMsg)
{
}

void UBeamGeneratorSubsystem::GenerateProduction_Ai()
{
	for (auto PedestalActor : AiPedestals)
	{
		if (!PedestalActor) continue;
		
		FString Key = PedestalActor->GetBeamStruct().BeamAssetsName;
		FBeamStruct* NewStruct = NewAiPedestalsRequestMap.Find(Key);
		
		if (NewStruct)
		{
			if (!PedestalActor->GetPedestalVisibility())
			{
				PedestalActor->ToggleVisibility(true);
			}
			const FBeamStruct* OldStruct = AiPedestalsRequestMap.Find(Key);
			if (!OldStruct || !OldStruct->IsExactlyEqual(*NewStruct))
			{
				PedestalActor->UpdateFromBeamStruct(*NewStruct);
			}
		}
		else
		{
			PedestalActor->ToggleVisibility(false);
		}
	}
}

void UBeamGeneratorSubsystem::GenerateProduction_Gjbz()
{
}

void UBeamGeneratorSubsystem::GenerateProduction_Zltz()
{
}

void UBeamGeneratorSubsystem::GenerateProduction_Cltz()
{
}

void UBeamGeneratorSubsystem::OnRequestAiProductionComplete(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                            const FString& StructName)
{
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("OnRequestAiProductionComplete::WebMsg %s"), *WebMsg));
	ProcessingProductionData_Ai(WebMsg);
}

void UBeamGeneratorSubsystem::OnRequestStaticProductionComplete(const FJsonStruct& JsonStruct, const FString& WebMsg,
                                                                const FString& StructName)
{
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("OnRequestStaticProductionComplete::WebMsg %s"), *WebMsg));

	if (StructName == TEXT("1"))
	{
		GenerateProduction_Gjbz();
	}
	else if (StructName == TEXT("2"))
	{
		GenerateProduction_Zltz();
	}
	else if (StructName == TEXT("3"))
	{
		GenerateProduction_Cltz();
	}
}
