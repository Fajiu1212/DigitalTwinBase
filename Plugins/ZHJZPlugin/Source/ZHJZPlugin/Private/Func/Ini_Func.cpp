// Fill out your copyright notice in the Description page of Project Settings.


#include "Func/Ini_Func.h"

#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
//#include "Misc/FileHelper.h"
//#include "Serialization/JsonSerializer.h"
//#include "Dom/JsonObject.h"

bool UIni_Func::ReadIniValueBool(const FString path, const FString type, const FString key)
{
	if (!GConfig) return false;
	FString FilePath = path;
	bool isExists= FFileManagerGeneric::Get().FileExists(*FilePath);
	if(!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	bool Value;
	
	GConfig->GetBool(*type, *key, Value, FilePath);

	return Value;
}

int UIni_Func::ReadIniValueInt(const FString path, const FString type, const FString key)
{
    if (!GConfig) return false;
	FString FilePath = path;
    int Value = -99999;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetInt(*type, *key, Value, FilePath);

    return Value;
}

FString UIni_Func::ReadIniValueString(const FString path, const FString type, const FString key)
{
	FString Value;
	if (!GConfig) return Value;
	FString FilePath = path;	
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetString(*type, *key, Value, FilePath);

	return Value;
}

float UIni_Func::ReadIniValueFloat(const FString path, const FString type, const FString key)
{
	if (!GConfig) return false;
	FString FilePath = path;
	float Value = 0.0f;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetFloat(*type, *key, Value, FilePath);

	return Value;
}

double UIni_Func::ReadIniValueDouble(const FString path, const FString type, const FString key)
{
	if (!GConfig) return false;
	FString FilePath = path;
	double Value = 0.0;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetDouble(*type, *key, Value, FilePath);

	return Value;
}

TArray<FString> UIni_Func::ReadIniValueArray(const FString path, const FString type, const FString key)
{
	TArray<FString>  Value;
	if (!GConfig) return Value;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetArray(*type, *key, Value, FilePath);

	return Value;
}

TArray<FString> UIni_Func::ReadIniValueSingleLineArray(const FString path, const FString type, const FString key)
{
	TArray<FString>  Value;
	if (!GConfig) return Value;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetSingleLineArray(*type, *key, Value, FilePath);

	return Value;
}

FColor UIni_Func::ReadIniValueColor(const FString path, const FString type, const FString key)
{
	FColor  Value;
	if (!GConfig) return Value;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetColor(*type, *key, Value, FilePath);

	return Value;
}

FVector2D UIni_Func::ReadIniValueVector2D(const FString path, const FString type, const FString key)
{
	FVector2D  Value;
	if (!GConfig) return Value;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetVector2D(*type, *key, Value, FilePath);

	return Value;
}

FVector UIni_Func::ReadIniValueVector(const FString path, const FString type, const FString key)
{
	FVector  Value;
	if (!GConfig) return Value;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetVector(*type, *key, Value, FilePath);

	return Value;
}

FVector4 UIni_Func::ReadIniValueVector4(const FString path, const FString type, const FString key)
{
	FVector4 Value;
	if (!GConfig) return Value;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetVector4(*type, *key, Value, FilePath);

	return Value;
}

FRotator UIni_Func::ReadIniValueRotator(const FString path, const FString type, const FString key)
{
	FRotator  Value;
	if (!GConfig) return Value;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetRotator(*type, *key, Value, FilePath);

	return Value;
}

TMap<FString, FString> UIni_Func::ReadIniValueMap(const FString path, const FString type, const FString key)
{
	TMap<FString, FString>  maps;
	if (!GConfig) return maps;
	FString FilePath = path;
	FString Value;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	GConfig->GetString(*type, *key, Value, FilePath);
	TArray<FString> ar;
	
	int32 length = Value.Len();
	FString lastValue= Value.Mid(1, length-2);
	lastValue.ParseIntoArray(ar, TEXT(","), true);
	for(int i=0;i<ar.Num();i++)
	{
		TArray<FString> ar2;
		ar[i].ParseIntoArray(ar2, TEXT("="), true);
		maps.Add(ar2[0],ar2[1]);
	}
	return maps;
}

bool UIni_Func::WriteIniValueBool(const FString path, const FString type, const FString key, const bool value)
{
	if (!GConfig) return false;
	FString FilePath = path;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetBool(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueInt(const FString path, const FString type, const FString key, const int value)
{
	if (!GConfig) return false;
	FString FilePath = path;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetInt(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueString(const FString path, const FString type, const FString key, const FString value)
{
	if (!GConfig) return false;
	FString FilePath = path;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetString(*type, *key, *value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueFloat(const FString path, const FString type, const FString key, const float value)
{
	if (!GConfig) return false;
	FString FilePath = path;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetFloat(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueDouble(const FString path, const FString type, const FString key, const double value)
{
	if (!GConfig) return false;
	FString FilePath = path;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetDouble(*type, *key, value, FilePath);
	return false;
}

bool UIni_Func::WriteIniValueArray(const FString path, const FString type, const FString key, const TArray<FString> value)
{
	if (!GConfig) return false;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetArray(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueSingleLineArray(const FString path, const FString type, const FString key, const TArray<FString> value)
{
	if (!GConfig) return false;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetSingleLineArray(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueColor(const FString path, const FString type, const FString key, const FColor value)
{
	if (!GConfig) return false;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetColor(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueVector2D(const FString path, const FString type, const FString key, const FVector2D value)
{
	if (!GConfig) return false;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetVector2D(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueVector(const FString path, const FString type, const FString key, const FVector value)
{
	if (!GConfig) return false;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetVector(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueVector4(const FString path, const FString type, const FString key, const FVector4 value)
{
	if (!GConfig) return false;
	FString FilePath = path;

	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		return false;
	}
	GConfig->SetVector4(*type, *key, value, FilePath);

	return true;
}

bool UIni_Func::WriteIniValueRotator(const FString path, const FString type, const FString key, const FRotator value)
{
	 if (!GConfig) return false;
	 FString FilePath = path;

	 bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	 if (!isExists)
	 {
		 //路径文件不存在 
		 return false;
	 }
	 GConfig->SetRotator(*type, *key, value, FilePath);

	 return true;
}

bool UIni_Func::WriteIniValueMap(const FString path, const FString type, const FString key, const TMap<FString, FString> value)
{
	TMap<FString, FString>  maps;
	if (!GConfig) return false;
	FString FilePath = path;
	FString str;
	bool isExists = FFileManagerGeneric::Get().FileExists(*FilePath);
	if (!isExists)
	{
		//路径文件不存在
		FilePath = FPaths::SourceConfigDir() + "Custom/CommonConfig.ini";
	}
	str.Append("(");
	for(auto&i:value)
	{
		str.Append(i.Key).Append("=").Append(i.Value).Append(",");
	}
	str.Append(")");
	GConfig->SetString(*type, *key, *str, FilePath);
	return true;
}

int32 UIni_Func::GetDaysByDataTime(FDateTime time1, FDateTime time2)
{
	int64 Timestamp1 = time1.ToUnixTimestamp();
	int64 Timestamp2 = time2.ToUnixTimestamp();
	int num;
	num = (Timestamp2 - Timestamp1)/ 86400;
	return num;
}

FDateTime UIni_Func::GetDataAfter(int32 num)
{
	FDateTime Now = FDateTime::Now();
	FDateTime Tomorrow = Now + FTimespan(num, 0, 0, 0);
	return Tomorrow;
}

//void UIni_Func::JsonObjectToString(const TSharedPtr<FJsonObject>& JsonObject, FString& OutString)
//{
//	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutString);
//	if (FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
//	{
//		// 序列化成功
//	}
//	else
//	{
//		// 序列化失败
//	}
//}

void  UIni_Func::SchedulePostHotloadRestart(const FString& OldPakFile)
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
}