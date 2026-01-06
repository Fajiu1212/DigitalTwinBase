// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manager/BaseManager.h"
#include "Common/EnumDefine.h"
#include "ConfigManager.generated.h"


/**
 * 
 */
UCLASS(Blueprintable)
class ZHJZPLUGIN_API UConfigManager : public UBaseManager
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
		void InitTestConfig();

	 bool GetIniConfigBool(const EProjType type, const FString key);

	int GetIniConfigInt(const EProjType type, const FString key);

	float GetIniConfigFloat(const EProjType type, const FString key);

	TArray<FString> GetIniConfigArray(const EProjType type, const FString key);

	TArray<FString> GetIniConfigSingleLineArray(const EProjType type, const FString key);

	FColor GetIniConfigColor(const EProjType type, const FString key);

	FVector2D GetIniConfigVector2D(const EProjType type, const FString key);

	FVector GetIniConfigVector(const EProjType type, const FString key);

	FVector4 GetIniConfigVector4(const EProjType type, const FString key);

	FRotator GetIniConfigRotator(const EProjType type, const FString key);

	//文件名
	 FString ConfigName = "Custom/CommonConfig.ini";

	//标题名
	FString TitleName = "Test";
	

};
