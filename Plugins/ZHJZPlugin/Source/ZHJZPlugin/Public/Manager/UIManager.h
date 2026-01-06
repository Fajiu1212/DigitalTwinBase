// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Common/StructDefine.h"
#include "BaseManager.h"
#include "UIManager.generated.h"

class UBase_UI;
/**
 * 
 */
UCLASS(Blueprintable)
class ZHJZPLUGIN_API UUIManager : public UBaseManager
{
	GENERATED_BODY()

protected:

	//打开UI
	UFUNCTION(BlueprintCallable)
		UBase_UI* OpenUI(const FString& UIName);

	//关闭UI，destory为true，从内存中销毁
	UFUNCTION(BlueprintCallable)
		bool CloseUI(const FString& UIName = "", bool destory = false);

	//得到UI
	UFUNCTION(BlueprintCallable)
		UBase_UI* GetUI(const FString& UIName);

	//创建UI(用于需要动态创建的子UI)
	UFUNCTION(BlueprintCallable)
		UBase_UI* CreateUI(const FString& UIName);

	//检测UI是否合法
	UFUNCTION(BlueprintCallable)
		bool CheckUI(const FString& UIName);

	//初始化UI
	UBase_UI* InitUI(const FString& UIName, bool isDestroy = false);

	// 当前存在的UI;UI名字：UI对象
	UPROPERTY(BlueprintReadOnly)
		TMap<FString, UBase_UI*> CurrentUIMap;

	//UI名字及其对应的路径 蓝图配置 
	UPROPERTY(BlueprintReadWrite)
		TMap<FString, FString> UIPathMap;

	//当前界面打开的UI
	UPROPERTY(BlueprintReadOnly)
		FCurrentUIMes CurrentWidgetMes;

	
};
