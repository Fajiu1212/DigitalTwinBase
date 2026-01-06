// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Ini_Func.generated.h"

/**
 * 
 */
UCLASS()
class ZHJZPLUGIN_API UIni_Func : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	//读取
	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool ReadIniValueBool(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static int ReadIniValueInt(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static FString ReadIniValueString(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static float ReadIniValueFloat(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static double ReadIniValueDouble(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static TArray<FString> ReadIniValueArray(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static TArray<FString> ReadIniValueSingleLineArray(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static FColor ReadIniValueColor(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static FVector2D ReadIniValueVector2D(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static FVector ReadIniValueVector(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static FVector4 ReadIniValueVector4(const FString path, const FString type, const FString key);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static FRotator ReadIniValueRotator(const FString path, const FString type, const FString key);

		//TMap结构
	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static TMap<FString, FString> ReadIniValueMap(const FString path, const FString type, const FString key);




		//写入
	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueBool(const FString path, const FString type, const FString key,const bool value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueInt(const FString path, const FString type, const FString key,const int value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueString(const FString path, const FString type, const FString key, const FString value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueFloat(const FString path, const FString type, const FString key, const float value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueDouble(const FString path, const FString type, const FString key, const double value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueArray(const FString path, const FString type, const FString key, const TArray<FString> value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueSingleLineArray(const FString path, const FString type, const FString key, const TArray<FString> value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueColor(const FString path, const FString type, const FString key, const FColor value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueVector2D(const FString path, const FString type, const FString key, const FVector2D value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueVector(const FString path, const FString type, const FString key, const FVector value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueVector4(const FString path, const FString type, const FString key, const FVector4 value);

	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueRotator(const FString path, const FString type, const FString key, const FRotator value);

		//TMap结构
	UFUNCTION(BlueprintCallable, Category = "Ini", meta = (WorldContext = "WorldContextObject"))
		static bool WriteIniValueMap(const FString path, const FString type, const FString key, const TMap<FString, FString> value);

		//计算两个时间相隔天数
	UFUNCTION(BlueprintCallable, Category = "Tool", meta = (WorldContext = "WorldContextObject"))
		static int32 GetDaysByDataTime(FDateTime time1, FDateTime time2);

	//获得n天后的日期
	UFUNCTION(BlueprintPure, Category = "Tool", meta = (WorldContext = "WorldContextObject"))
		static  FDateTime GetDataAfter(int32 num);

		//热更新后清理重启
	UFUNCTION(BlueprintCallable, Category = "Tool", meta = (WorldContext = "WorldContextObject"))
		static void SchedulePostHotloadRestart(const FString& OldPakFile);

};

