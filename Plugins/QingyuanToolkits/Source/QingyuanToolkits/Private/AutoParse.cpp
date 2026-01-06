#include "AutoParse.h"

#include "HttpModule.h"
#include "IPlatformFilePak.h"
#include "JsonObjectConverter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Engine/AssetManager.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Interfaces/IHttpResponse.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

UAutoParse* UAutoParse::AutoHttpRequest(const FString& URL, const FString& JsonContent, const FString& VerbType,
                                        const TMap<FString, FString> HeadMap, const FString& StructName,
                                        ERequestType InDataType)
{
	UAutoParse* Instance = NewObject<UAutoParse>();
	if (Instance)
	{
		Instance->LocalStr = StructName;
		Instance->RequestType = InDataType;
		if (InDataType == ERequestType::HttpOnly || InDataType == ERequestType::Auto)
		{
			Instance->LoadOnlineData(URL, JsonContent, VerbType, HeadMap);
		}
		else if (InDataType == ERequestType::LocalOnly)
		{
			Instance->LoadLocalData();
		}
	}
	return Instance;
}

void UAutoParse::LoadLocalData()
{
	//待完善
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		FJsonStruct JsonStruct;
		FString WebMsg;
		//Temp
		OnSuccess.Broadcast(JsonStruct, WebMsg, LocalStr);
	});
}

void UAutoParse::LoadOnlineData(const FString& URL, const FString& JsonContent, const FString& VerbType,
                                const TMap<FString, FString> HeadMap)
{
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	if (Request)
	{
		Request->SetURL(URL);
		Request->SetVerb(VerbType);
		Request->SetHeader("Content-Type", "application/json;charset=utf-8");
		for (auto Item : HeadMap)
		{
			Request->SetHeader(Item.Key, Item.Value);
		}
		if (!JsonContent.IsEmpty())
		{
			Request->SetContentAsString(JsonContent);
		}
		HeadMapReceived.Empty();
		Request->OnHeaderReceived().BindUObject(this, &UAutoParse::OnResponseHeadersReceived);
		Request->OnProcessRequestComplete().BindUObject(this, &UAutoParse::OnRequestComplete);

		Request->ProcessRequest();
	}
}

void UAutoParse::OnResponseHeadersReceived(FHttpRequestPtr Request, const FString& HeaderName,
                                           const FString& NewHeaderValue)
{
	HeadMapReceived.Add(HeaderName, NewHeaderValue);
}

void UAutoParse::OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	FJsonStruct JsonStruct;
	if (!Response.IsValid())
	{
		if (RequestType == ERequestType::Auto)
		{
			LoadLocalData();
		}
		else
		{
			OnFail.Broadcast(JsonStruct, "Response invalid!", LocalStr);
		}
		return;
	}

	int32 Code = Response->GetResponseCode();
	FString OutputStr = Response->GetContentAsString();

	if (!bWasSuccessful)
	{
		if (RequestType == ERequestType::Auto)
		{
			LoadLocalData();
		}
		else
		{
			OnFail.Broadcast(JsonStruct, "bWasSuccessful == false!", LocalStr);
		}
		return;
	}
	if (!EHttpResponseCodes::IsOk(Code))
	{
		if (RequestType == ERequestType::Auto)
		{
			LoadLocalData();
		}
		else
		{
			OnFail.Broadcast(JsonStruct, "Bade code!", LocalStr);
		}
		return;
	}
	else
	{
		FString ObjName = TEXT("ConfigFile/SeverMsg/") + LocalStr + TEXT(".json");
		FString FilePath = FPaths::ProjectSavedDir() / ObjName;
		SaveStrByPath(FilePath, OutputStr);
		FString StructJsonStr = TEXT("{ \"") + LocalStr + TEXT("\":") + OutputStr + TEXT("}");
		FJsonObjectConverter::JsonObjectStringToUStruct(StructJsonStr, &JsonStruct, 0, 0);
		JsonStruct.HttpHeadReceived.HeadReceived = HeadMapReceived;
		OnSuccess.Broadcast(JsonStruct, OutputStr, LocalStr);
	}
}

bool UAutoParse::LoadDataFromLocalFile(const FString InFileName, const FString InStructName, FString& OutString,
                                       FJsonStruct& OutJsonStruct)
{
	if (FPaths::FileExists(InFileName) == false)
	{
		return false;
	}

	bool bLoadSucceed = FFileHelper::LoadFileToString(OutString, *InFileName);
	if (bLoadSucceed == false)
	{
		return false;
	}
	FString StructJsonStr = TEXT("{ \"") + InStructName + TEXT("\":") + OutString + TEXT("}");
	//JsonStrToStruct(StructJsonStr, OutJsonStruct);
	return true;
}

void UAutoParse::SaveStrByPath(const FString OutFileName, const FString SaveStr)
{
	FFileHelper::SaveStringToFile(SaveStr, *OutFileName);
}

bool UAutoParse::GetJsonObjectFromJsonStr(const FString& In_JsonStr, FAutoParseJsonObject& Out_JsonObject)
{
	TSharedRef<TJsonReader<TCHAR>> TheJsonReader = TJsonReaderFactory<TCHAR>::Create(In_JsonStr);
	bool bSucceed = FJsonSerializer::Deserialize(TheJsonReader, Out_JsonObject.AutoParseJsonObject);
	return bSucceed;
}

bool UAutoParse::GetJsonObjectFromLocalFile(const FString& In_FilePath, FAutoParseJsonObject& Out_JsonObject)
{
	if (FPaths::FileExists(In_FilePath) == false)
	{
		const FString LogStr = TEXT("Find Path") + In_FilePath + TEXT("Failed");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetJsonObjectFromLocalFile Warning: %s"), *LogStr);
		return false;
	}
	FString JsonStr;
	bool bSucceed = FFileHelper::LoadFileToString(JsonStr, *In_FilePath);
	if (bSucceed == false)
	{
		FString LogStr = TEXT("Load Path") + In_FilePath + TEXT("ToStr Failed");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetJsonObjectFromLocalFile Warning: %s"), *LogStr);
		return false;
	}
	TSharedRef<TJsonReader<TCHAR>> TheJsonReader = TJsonReaderFactory<TCHAR>::Create(JsonStr);
	bool bOk = FJsonSerializer::Deserialize(TheJsonReader, Out_JsonObject.AutoParseJsonObject);
	return bOk;
}

bool UAutoParse::JsonObjectToJsonStr(FAutoParseJsonObject In_JsonObject, FString& Out_JsonStr)
{
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out_JsonStr);
	bool bSucceed = FJsonSerializer::Serialize(In_JsonObject.AutoParseJsonObject.ToSharedRef(), Writer);
	return bSucceed;
}

bool UAutoParse::SetObjectValueFormStr(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                       const FString& In_Value,
                                       FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject is NULL!");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}

	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		FAutoParseJsonObject TempJsonStruct;
		FString TempJsonStr = TEXT("{\"") + In_StrFile + TEXT("\":") + In_Value + TEXT("}");
		bool bflag = GetJsonObjectFromJsonStr(TempJsonStr, TempJsonStruct);
		Out_JsonObject.AutoParseJsonObject->SetObjectField(In_StrFile, TempJsonStruct.AutoParseJsonObject);
		return bflag;
	}
	else
	{
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetObjectValueFormStr(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::GetStrValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                       FString& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->TryGetStringField(In_StrFile, Out_Value))
		{
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->TryGetStringField(SeparatedStrings[i], Out_Value))
					{
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}

	return false;
}

bool UAutoParse::GetStrArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                            TArray<FString>& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->TryGetStringArrayField(In_StrFile, Out_Value))
		{
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->TryGetStringArrayField(SeparatedStrings[i], Out_Value))
					{
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}

	return false;
}

bool UAutoParse::SetStrValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                       FString In_Value,
                                       FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	//const int32 nArraySize = In_StrFile.ParseIntoArray(SeparatedStrings, TEXT("."), true);
	if (SeparatedStrings.Num() == 1)
	{
		Out_JsonObject.AutoParseJsonObject->SetStringField(In_StrFile, In_Value);
		return true;
	}
	else
	{
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetStrValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::SetStrArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                            TArray<FString> In_Value, FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValue;
		for (auto Value : In_Value)
		{
			FJsonValueString TempValue(Value);
			TSharedPtr<FJsonValue> PtrValue = MakeShared<FJsonValueString>(TempValue);
			JsonValue.Add(PtrValue);
		}
		Out_JsonObject.AutoParseJsonObject->SetArrayField(In_StrFile, JsonValue);
		return true;
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetStrArrayValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::GetInt32ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                         int32& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt32ValueFormObject Warning: %s"), *LogStr);
		return false;
	}

	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->HasField(In_StrFile))
		{
			Out_Value = In_JsonObject.AutoParseJsonObject->GetIntegerField(In_StrFile);
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt32ValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						Out_Value = TargetJsonObject->GetIntegerField(SeparatedStrings[i]);
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt32ValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt32ValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt32ValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}

	return false;
}

bool UAutoParse::GetInt32ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                              TArray<int32>& Out_Value)
{
	return GetArrayValueFormObject(In_StrFile, In_JsonObject, Out_Value);
}

bool UAutoParse::SetInt32ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                         int32 In_Value,
                                         FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		Out_JsonObject.AutoParseJsonObject->SetNumberField(In_StrFile, In_Value);
		return true;
	}
	else
	{
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetInt32ValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::SetInt32ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                              TArray<int32> In_Value, FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValue;
		for (auto Value : In_Value)
		{
			FJsonValueNumber TempValue(Value);
			TSharedPtr<FJsonValue> PtrValue = MakeShared<FJsonValueNumber>(TempValue);
			JsonValue.Add(PtrValue);
		}
		Out_JsonObject.AutoParseJsonObject->SetArrayField(In_StrFile, JsonValue);
		return true;
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetInt32ArrayValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::GetInt64ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                         int64& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt64ValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->HasField(In_StrFile))
		{
			Out_Value = In_JsonObject.AutoParseJsonObject->GetNumberField(In_StrFile);
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt64ValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						Out_Value = TargetJsonObject->GetNumberField(SeparatedStrings[i]);
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt64ValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt64ValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetInt64ValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	return false;
}

bool UAutoParse::GetInt64ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                              TArray<int64>& Out_Value)
{
	return GetArrayValueFormObject(In_StrFile, In_JsonObject, Out_Value);
}

bool UAutoParse::SetInt64ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                         int64 In_Value,
                                         FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		Out_JsonObject.AutoParseJsonObject->SetNumberField(In_StrFile, In_Value);
		return true;
	}
	else
	{
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetInt64ValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::SetInt64ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                              TArray<int64> In_Value, FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValue;
		for (auto Value : In_Value)
		{
			FJsonValueNumber TempValue(Value);
			TSharedPtr<FJsonValue> PtrValue = MakeShared<FJsonValueNumber>(TempValue);
			JsonValue.Add(PtrValue);
		}
		Out_JsonObject.AutoParseJsonObject->SetArrayField(In_StrFile, JsonValue);
		return true;
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetInt64ArrayValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::GetFloatValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                         float& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetFloatValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->HasField(In_StrFile))
		{
			Out_Value = In_JsonObject.AutoParseJsonObject->GetNumberField(In_StrFile);
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetFloatValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						Out_Value = TargetJsonObject->GetNumberField(SeparatedStrings[i]);
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetFloatValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetFloatValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetFloatValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	return false;
}

bool UAutoParse::GetFloatArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                              TArray<float>& Out_Value)
{
	return GetArrayValueFormObject(In_StrFile, In_JsonObject, Out_Value);
}

bool UAutoParse::SetFloatValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                         float In_Value,
                                         FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		Out_JsonObject.AutoParseJsonObject->SetNumberField(In_StrFile, In_Value);
		return true;
	}
	else
	{
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetFloatValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::SetFloatArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                              TArray<float> In_Value, FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValue;
		for (auto Value : In_Value)
		{
			FJsonValueNumber TempValue(Value);
			TSharedPtr<FJsonValue> PtrValue = MakeShared<FJsonValueNumber>(TempValue);
			JsonValue.Add(PtrValue);
		}
		Out_JsonObject.AutoParseJsonObject->SetArrayField(In_StrFile, JsonValue);
		return true;
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetFloatArrayValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::GetBoolValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                        bool& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetBoolValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->HasField(In_StrFile))
		{
			Out_Value = In_JsonObject.AutoParseJsonObject->GetBoolField(In_StrFile);
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetBoolValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						Out_Value = TargetJsonObject->GetBoolField(SeparatedStrings[i]);
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetBoolValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetBoolValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetBoolValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	return false;
}

bool UAutoParse::GetBoolArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                             TArray<bool>& Out_Value)
{
	return GetArrayValueFormObject(In_StrFile, In_JsonObject, Out_Value);
}

bool UAutoParse::SetBoolValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                        bool In_Value,
                                        FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		Out_JsonObject.AutoParseJsonObject->SetBoolField(In_StrFile, In_Value);
		return true;
	}
	else
	{
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetBoolValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::SetBoolArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                             TArray<bool> In_Value, FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValue;
		for (auto Value : In_Value)
		{
			FJsonValueBoolean TempValue(Value);
			TSharedPtr<FJsonValue> PtrValue = MakeShared<FJsonValueBoolean>(TempValue);
			JsonValue.Add(PtrValue);
		}
		Out_JsonObject.AutoParseJsonObject->SetArrayField(In_StrFile, JsonValue);
		return true;
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetBoolArrayValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::GetObjValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                       FAutoParseJsonObject& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->HasField(In_StrFile))
		{
			Out_Value.AutoParseJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(In_StrFile);
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						Out_Value.AutoParseJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	return false;
}

bool UAutoParse::GetObjArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                            TArray<FAutoParseJsonObject>& Out_Value)
{
	if (!In_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		if (In_JsonObject.AutoParseJsonObject->HasField(In_StrFile))
		{
			TArray<FAutoParseJsonObject> TempObjectArray;
			TArray<TSharedPtr<FJsonValue>> TempValueArray;
			TempValueArray = In_JsonObject.AutoParseJsonObject->GetArrayField(In_StrFile);
			for (auto Value : TempValueArray)
			{
				FAutoParseJsonObject VJson;
				VJson.AutoParseJsonObject = Value->AsObject();
				TempObjectArray.Add(VJson);
			}
			Out_Value = TempObjectArray;
			return true;
		}
		else
		{
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TArray<FAutoParseJsonObject> TempObjectArray;
						TArray<TSharedPtr<FJsonValue>> TempValueArray;
						TempValueArray = In_JsonObject.AutoParseJsonObject->GetArrayField(SeparatedStrings[i]);
						for (auto Value : TempValueArray)
						{
							FAutoParseJsonObject VJson;
							VJson.AutoParseJsonObject = Value->AsObject();
							TempObjectArray.Add(VJson);
						}
						Out_Value = TempObjectArray;
						return true;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjArrayValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
				else
				{
					TSharedPtr<FJsonObject> TempJsonObject;
					if (TargetJsonObject->HasField(SeparatedStrings[i]))
					{
						TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
						TargetJsonObject = TempJsonObject;
					}
					else
					{
						FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
						UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjArrayValueFormObject Warning: %s"), *LogStr);
						return false;
					}
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetObjArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
	return false;
}

bool UAutoParse::SetObjValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                       FAutoParseJsonObject In_Value, FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		Out_JsonObject.AutoParseJsonObject->SetObjectField(In_StrFile, In_Value.AutoParseJsonObject);
		return true;
	}
	else
	{
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetObjValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

bool UAutoParse::SetObjArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
                                            TArray<FAutoParseJsonObject> In_Value, FAutoParseJsonObject& Out_JsonObject)
{
	Out_JsonObject = In_JsonObject;
	if (!Out_JsonObject.AutoParseJsonObject)
	{
		FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
		UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
		return false;
	}
	TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
	if (SeparatedStrings.Num() == 1)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValue;
		for (auto Value : In_Value)
		{
			FJsonValueObject TempValue(Value.AutoParseJsonObject);
			TSharedPtr<FJsonValue> PtrValue = MakeShared<FJsonValueObject>(TempValue);
			JsonValue.Add(PtrValue);
		}
		Out_JsonObject.AutoParseJsonObject->SetArrayField(In_StrFile, JsonValue);
		return true;
	}
	else
	{
		TSharedPtr<FJsonObject> TargetJsonObject;
		if (Out_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			FString TempStrFile;
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (TempStrFile.IsEmpty())
				{
					TempStrFile = SeparatedStrings[i];
				}
				else
				{
					TempStrFile = TempStrFile + TEXT(".") + SeparatedStrings[i];
				}
			}

			FAutoParseJsonObject TempJsonStruct;
			TempJsonStruct.AutoParseJsonObject = Out_JsonObject.AutoParseJsonObject->
			                                                    GetObjectField(SeparatedStrings[0]);
			bool tFlag = SetObjArrayValueFormObject(TempStrFile, TempJsonStruct, In_Value, TempJsonStruct);
			Out_JsonObject.AutoParseJsonObject->SetObjectField(SeparatedStrings[0], TempJsonStruct.AutoParseJsonObject);
			return tFlag;
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetStrArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}
	}
}

void UAutoParse::PrintLog_GameThread(FString InStr)
{
	AsyncTask(ENamedThreads::Type::GameThread, [InStr]()
	{
		UE_LOG(LogTemp, Warning, TEXT("ThreadLog:[%s]"), *InStr)
	});
}

bool UAutoParse::GetMsgValueByKeyStr(const FString& In_Message, const FString& In_Key,
                                     FAutoParseJsonObject& Out_JsonObject)
{
	TSharedPtr<FJsonValue> JsonValue;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(In_Message);

	if (FJsonSerializer::Deserialize(JsonReader, JsonValue) && JsonValue.IsValid())
	{
		const TArray<TSharedPtr<FJsonValue>>* JsonArray;
		if (JsonValue->TryGetArray(JsonArray) && JsonArray->Num() > 0)
		{
			for (auto ArrObject : *JsonArray)
			{
				if (ArrObject->AsObject())
				{
					FString KeyValue = FString();
					if (ArrObject->AsObject()->TryGetStringField(TEXT("key"), KeyValue) && KeyValue == In_Key)
					{
						const TSharedPtr<FJsonObject>* ValueObj;
						if (ArrObject->AsObject()->TryGetObjectField(TEXT("value"), ValueObj) && ValueObj->IsValid())
						{
							Out_JsonObject.AutoParseJsonObject = *ValueObj;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

float UAutoParse::ConvertFontFromPxToPt(float PixelSize)
{
	TSharedPtr<SWindow> MainWindow = FSlateApplication::Get().GetActiveTopLevelWindow();

	FVector2D WindowCenter = MainWindow->GetPositionInScreen() + MainWindow->GetSizeInScreen() * 0.5f;

	float DPIScale = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(
		WindowCenter.X,
		WindowCenter.Y);

	return PixelSize * 72.0f / (96.0f * DPIScale);
}

void UAutoParse::RefreshAndPreloadPakAssets(const FString& MountPoint)
{
	FSoftObjectPath AssetPath(MountPoint);
	AssetPath.FixupForPIE();
	UObject* AssetObject = AssetPath.TryLoad();
	if (AssetObject != nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded asset object %s"), *AssetObject->GetName());
	}

	FString PakFilePath = MountPoint;
	FString PakMountDirectory = FPaths::GetBaseFilename(PakFilePath);

	PakMountDirectory = "/" + PakMountDirectory + "/";
	IPlatformFile* LocalPlatformFile = &FPlatformFileManager::Get().GetPlatformFile();
	if (LocalPlatformFile != nullptr)
	{
		IPlatformFile* PakPlatformFile = FPlatformFileManager::Get().GetPlatformFile(TEXT("PakFile"));
		FPakPlatformFile* PakPlatform = new FPakPlatformFile();
		PakPlatform->Initialize(LocalPlatformFile, TEXT(""));
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatform);

		FPakFile* PakFile = new FPakFile(LocalPlatformFile, *PakFilePath, false);
		PakFile->SetMountPoint(*PakMountDirectory);
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByPath(FName(*("/" + FPaths::GetBaseFilename(PakFilePath))), AssetData, true,
	                                          false);
	for (FAssetData Asset : AssetData)
	{
		FString Path = Asset.GetObjectPathString();
		FSoftObjectPath AssetPathCheck(Path);
		AssetPath.FixupForPIE();
		UObject* AssetObjectCheck = AssetPathCheck.TryLoad();
	}
}

bool UAutoParse::UpdatePak(const FString& PakFileName)
{
	const FString AbsolutePakPath = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectContentDir() / "Paks" / PakFileName
	);


	if (!FPaths::FileExists(AbsolutePakPath))
	{
		return false;
	}


	IPlatformFile* CurrentPlatformFile = &FPlatformFileManager::Get().GetPlatformFile();
	FPakPlatformFile* PakPlatformFile = static_cast<FPakPlatformFile*>(
		FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile"))
	);

	if (!PakPlatformFile)
	{
		PakPlatformFile = new FPakPlatformFile();
		PakPlatformFile->Initialize(CurrentPlatformFile, TEXT(""));
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
	}

	const FString MountPoint = FPaths::ProjectContentDir() + TEXT("Paks/");
	TArray<FString> MountedPaks;
	PakPlatformFile->GetMountedPakFilenames(MountedPaks);
	if (MountedPaks.Contains(AbsolutePakPath))
	{
		PakPlatformFile->Unmount(*AbsolutePakPath);
	}

	if (!PakPlatformFile->Mount(*AbsolutePakPath, 100, *MountPoint))
	{
		return false;
	}
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.SearchAllAssets(true);

	return true;
}

bool UAutoParse::IsPackagedMode()
{
	ULocalPlayer* LocalPlayer = GEngine->FindFirstLocalPlayerFromControllerId(0);

	if (!LocalPlayer)
	{
		return true;
	}

	UWorld* World = LocalPlayer->GetWorld();

	if (World)
	{
		if (World->GetNetMode() == ENetMode::NM_DedicatedServer)
		{
			return true;
		}

		if (World->WorldType == EWorldType::Editor ||
			World->WorldType == EWorldType::EditorPreview ||
			World->WorldType == EWorldType::PIE)
		{
			return false;
		}
	}

	return true;
}

FTimerHandle UAutoParse::SmoothFloatInterp(
	UObject* WorldContextObject,
	float Curr,
	float Target,
	float Duration,
	float Step,
	const FOnFloatValueDelegate& OnTick,
	const FOnFloatValueDelegate& OnComplete)
{
	FTimerHandle OutTimerHandle;
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		if (OnComplete.IsBound())
		{
			OnComplete.Execute(Target);
		}
		return OutTimerHandle;
	}

	UWorld* World = WorldContextObject->GetWorld();

	struct FInterpData
	{
		float CurrentValue;
		float TargetValue;
		float TimeElapsed;
		float TotalDuration;
		float StepSize;
		float ValuePerStep;
		FOnFloatValueDelegate TickCallback;
		FOnFloatValueDelegate CompleteCallback;
		FTimerHandle TimerHandle;

		FInterpData(float InCurr, float InTarget, float InDuration, float InStep)
			: CurrentValue(InCurr)
			  , TargetValue(InTarget)
			  , TimeElapsed(0.0f)
			  , TotalDuration(FMath::Max(InDuration, KINDA_SMALL_NUMBER))
			  , StepSize(FMath::Max(InStep, KINDA_SMALL_NUMBER))
		{
			int32 Steps = FMath::Max(1, FMath::RoundToInt(TotalDuration / StepSize));
			ValuePerStep = (TargetValue - CurrentValue) / Steps;
		}

		bool Update(float& OutValue)
		{
			if (TimeElapsed >= TotalDuration)
			{
				OutValue = TargetValue;
				return false;
			}

			CurrentValue += ValuePerStep;
			TimeElapsed += StepSize;

			if ((ValuePerStep > 0.0f && CurrentValue > TargetValue) ||
				(ValuePerStep < 0.0f && CurrentValue < TargetValue))
			{
				CurrentValue = TargetValue;
			}

			OutValue = CurrentValue;
			return TimeElapsed < TotalDuration;
		}
	};

	TSharedPtr<FInterpData> InterpData = MakeShared<FInterpData>(Curr, Target, Duration, Step);
	InterpData->TickCallback = OnTick;
	InterpData->CompleteCallback = OnComplete;

	FTimerHandle& TimerHandleRef = OutTimerHandle;
	auto TimerCallback = [InterpData, World, &TimerHandleRef]()
	{
		float Value;
		bool bContinue = InterpData->Update(Value);

		if (InterpData->TickCallback.IsBound())
		{
			InterpData->TickCallback.Execute(Value);
		}

		if (!bContinue)
		{
			if (InterpData->CompleteCallback.IsBound())
			{
				InterpData->CompleteCallback.Execute(Value);
			}

			World->GetTimerManager().ClearTimer(TimerHandleRef);
		}
	};

	World->GetTimerManager().SetTimer(
		OutTimerHandle,
		FTimerDelegate::CreateLambda(TimerCallback),
		Step,
		true);
	return OutTimerHandle;
}


FVector2D UAutoParse::MeasureDesiredSize(APlayerController* OptionalOwningPlayer, TSubclassOf<UUserWidget> Widget)
{
	if (!Widget) return FVector2D::Zero();

	UUserWidget* W = CreateWidget<UUserWidget>(OptionalOwningPlayer, Widget);
	if (!W) return FVector2D::Zero();

	if (!W->GetCachedWidget().IsValid())
	{
		(void)W->TakeWidget();
	}
	
	W->ForceLayoutPrepass();
	return W->GetDesiredSize();
}
