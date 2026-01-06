#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Interfaces/IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AutoParse.generated.h"

UENUM()
enum class ERequestType : uint8
{
	LocalOnly,
	HttpOnly,
	Auto
};

USTRUCT(BlueprintType)
struct FAutoParseJsonObject
{
	GENERATED_USTRUCT_BODY()

	TSharedPtr<FJsonObject> AutoParseJsonObject;
};

USTRUCT(BlueprintType)
struct FCmdSetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> CmdStrArray;
};

USTRUCT(BlueprintType)
struct FHttpHeadReceived
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> HeadReceived;
};

USTRUCT(BlueprintType)
struct FBeamMeshDataTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString BeamMeshName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BeamMeshLength;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* BeamMeshClass;
};

USTRUCT(BlueprintType)
struct FBeamMesh
{
	GENERATED_BODY()

public:
	FBeamMesh()
	{
	}

	FBeamMesh(const FString& InBeamMeshName, UStaticMesh* InBeamMeshClass) : BeamMeshName(InBeamMeshName),
	                                                                         BeamMeshClass(InBeamMeshClass)
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString BeamMeshName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMesh* BeamMeshClass = nullptr;
};

USTRUCT(BlueprintType)
struct FAnimation2DTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UTexture2D* Animation2D;
};

USTRUCT(BlueprintType)
struct FLoginUserData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AutoParse")
	FString UserName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AutoParse")
	FString Password;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AutoParse")
	bool bSaved;
};

USTRUCT(BlueprintType)
struct FJsonStruct
{
	GENERATED_BODY()

#pragma region Default Structs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCmdSetData CmdSetData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FHttpHeadReceived HttpHeadReceived;
#pragma endregion
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFloatValueDelegate, float, Value);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAutoParseDelegate, const FJsonStruct&, JsonStruct, const FString&,
                                               WebMsg, const FString&, StructName);

UCLASS()
class QINGYUANTOOLKITS_API UAutoParse : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

#pragma region HttpRequest

public:
	UFUNCTION(BlueprintCallable, Category = "AwsaToolKits|ToolKits")
	static UAutoParse* AutoHttpRequest(const FString& URL, const FString& JsonContent, const FString& VerbType,
	                                   const TMap<FString, FString> HeadMap, const FString& StructName,
	                                   ERequestType InDataType = ERequestType::Auto);
	UPROPERTY(BlueprintAssignable, Category = "AwsaToolKits|ToolKits")
	FAutoParseDelegate OnSuccess;
	UPROPERTY(BlueprintAssignable, Category = "AwsaToolKits|ToolKits")
	FAutoParseDelegate OnFail;

private:
	void LoadLocalData();
	void LoadOnlineData(const FString& URL, const FString& JsonContent, const FString& VerbType,
	                    const TMap<FString, FString> HeadMap);

	void OnResponseHeadersReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue);
	void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FString LocalStr;
	ERequestType RequestType;
	TMap<FString, FString> HeadMapReceived;
#pragma endregion
#pragma region Local File
	//Local file read & write
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool LoadDataFromLocalFile(const FString InFileName, const FString InStructName, FString& OutString,
	                                  FJsonStruct& OutJsonStruct);
	UFUNCTION(BlueprintCallable, Category = "AwsaToolKits|ToolKits")
	static void SaveStrByPath(FString OutFileName, FString SaveStr);
#pragma endregion
#pragma region Json数据处理

protected:
	static TArray<FString> SplitJsonPath(const FString& Path)
	{
		TArray<FString> Parts;
		Path.ParseIntoArray(Parts, TEXT("."), true);
		return Parts;
	}

public:
	//JsonStr转换JsonObject
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetJsonObjectFromJsonStr(const FString& In_JsonStr, FAutoParseJsonObject& Out_JsonObject);

	//通过本地文件获取JsonObject
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetJsonObjectFromLocalFile(const FString& In_FilePath, FAutoParseJsonObject& Out_JsonObject);

	//JsonObjectToStr
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool JsonObjectToJsonStr(FAutoParseJsonObject In_JsonObject, FString& Out_JsonStr);

	//Obj
	//设置Obj值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetObjectValueFormStr(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                  const FString& In_Value,
	                                  FAutoParseJsonObject& Out_JsonObject);

	//Str
	//获取Str值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetStrValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                  FString& Out_Value);
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetStrArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                       TArray<FString>& Out_Value);
	//设置Str值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetStrValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                  FString In_Value,
	                                  FAutoParseJsonObject& Out_JsonObject);
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetStrArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                       TArray<FString> In_Value, FAutoParseJsonObject& Out_JsonObject);

	//Int32
	//获取Int32值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetInt32ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                    int32& Out_Value);
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetInt32ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                         TArray<int32>& Out_Value);
	//设置Int32值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetInt32ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                    int32 In_Value,
	                                    FAutoParseJsonObject& Out_JsonObject);
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetInt32ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                         TArray<int32> In_Value, FAutoParseJsonObject& Out_JsonObject);

	//Int64
	//获取Int64值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetInt64ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                    int64& Out_Value);
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetInt64ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                         TArray<int64>& Out_Value);
	//设置Int64值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetInt64ValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                    int64 In_Value,
	                                    FAutoParseJsonObject& Out_JsonObject);

	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetInt64ArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                         TArray<int64> In_Value, FAutoParseJsonObject& Out_JsonObject);

	//Float
	//获取Float值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetFloatValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                    float& Out_Value);

	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetFloatArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                         TArray<float>& Out_Value);
	//设置Float值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetFloatValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                    float In_Value,
	                                    FAutoParseJsonObject& Out_JsonObject);

	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetFloatArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                         TArray<float> In_Value, FAutoParseJsonObject& Out_JsonObject);


	//Bool
	//获取Bool值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetBoolValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                   bool& Out_Value);

	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetBoolArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                        TArray<bool>& Out_Value);
	//设置Bool值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetBoolValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                   bool In_Value,
	                                   FAutoParseJsonObject& Out_JsonObject);

	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetBoolArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                        TArray<bool> In_Value, FAutoParseJsonObject& Out_JsonObject);

	//Object
	//获取Object值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetObjValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                  FAutoParseJsonObject& Out_Value);

	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool GetObjArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                       TArray<FAutoParseJsonObject>& Out_Value);
	//设置Object值
	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetObjValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                  FAutoParseJsonObject In_Value, FAutoParseJsonObject& Out_JsonObject);

	UFUNCTION(BlueprintPure, Category = "AwsaToolKits|ToolKits")
	static bool SetObjArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                       TArray<FAutoParseJsonObject> In_Value, FAutoParseJsonObject& Out_JsonObject);

	template <typename TEnum>
	static bool GetArrayValueFormObject(const FString& In_StrFile, const FAutoParseJsonObject& In_JsonObject,
	                                    TArray<TEnum>& Out_Value)
	{
		if (!In_JsonObject.AutoParseJsonObject)
		{
			FString LogStr = TEXT("In_JsonObject.AutoParseJsonObject Is NULL");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}

		TArray<FString> SeparatedStrings = SplitJsonPath(In_StrFile);
		//const int32 nArraySize = In_StrFile.ParseIntoArray(SeparatedStrings, TEXT("."), true);

		if (SeparatedStrings.Num() == 1)
		{
			if (In_JsonObject.AutoParseJsonObject->TryGetEnumArrayField(In_StrFile, Out_Value))
			{
				return true;
			}
			FString LogStr = TEXT("Find File") + In_StrFile + TEXT("Failed");
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}

		TSharedPtr<FJsonObject> TargetJsonObject;
		if (In_JsonObject.AutoParseJsonObject->HasField(SeparatedStrings[0]))
		{
			TargetJsonObject = In_JsonObject.AutoParseJsonObject->GetObjectField(SeparatedStrings[0]);
			for (size_t i = 1; i < SeparatedStrings.Num(); i++)
			{
				if (i == SeparatedStrings.Num() - 1)
				{
					if (TargetJsonObject->TryGetEnumArrayField(SeparatedStrings[i], Out_Value))
					{
						return true;
					}
					FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
					UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetArrayValueFormObject Warning: %s"), *LogStr);
					return false;
				}
				TSharedPtr<FJsonObject> TempJsonObject;
				if (TargetJsonObject->HasField(SeparatedStrings[i]))
				{
					TempJsonObject = TargetJsonObject->GetObjectField(SeparatedStrings[i]);
					TargetJsonObject = TempJsonObject;
				}
				else
				{
					FString LogStr = TEXT("Find File") + SeparatedStrings[i] + TEXT("Failed From") + In_StrFile;
					UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetArrayValueFormObject Warning: %s"), *LogStr);
					return false;
				}
			}
		}
		else
		{
			FString LogStr = TEXT("Find File") + SeparatedStrings[0] + TEXT("Failed From") + In_StrFile;
			UE_LOG(LogTemp, Warning, TEXT("UAutoParse::GetArrayValueFormObject Warning: %s"), *LogStr);
			return false;
		}

		return false;
	}
#pragma endregion
#pragma region Default
	UFUNCTION(BlueprintCallable, Category="AwsaToolKits|ToolKits")
	static void PrintLog_GameThread(FString InStr);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="AwsaToolKits|ToolKits")
	static bool GetMsgValueByKeyStr(const FString& In_Message, const FString& In_Key,
	                                FAutoParseJsonObject& Out_JsonObject);
#if PLATFORM_WINDOWS
	static float ConvertFontFromPxToPt(float InPx);
#endif
	UFUNCTION(BlueprintCallable, Category="AwsaToolKits|ToolKits")
	static void RefreshAndPreloadPakAssets(const FString& MountPoint);

	UFUNCTION(BlueprintCallable, Category="AwsaToolKits|ToolKits")
	static bool UpdatePak(const FString& PakFileName);

	UFUNCTION(BlueprintPure, Category="AwsaToolKits|ToolKits")
	static bool IsPackagedMode();
	UFUNCTION(BlueprintCallable, Category = "AwsaToolKits|ToolKits",
		meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "false"))
	static FTimerHandle SmoothFloatInterp(
		UObject* WorldContextObject,
		float Curr,
		float Target,
		float Duration,
		float Step,
		const FOnFloatValueDelegate& OnTick,
		const FOnFloatValueDelegate& OnComplete);
	
	UFUNCTION(BlueprintCallable, Category = "AwsaToolKits|ToolKits")
	static FVector2D MeasureDesiredSize(APlayerController* OptionalOwningPlayer, TSubclassOf<UUserWidget> Widget);

#pragma endregion
};
