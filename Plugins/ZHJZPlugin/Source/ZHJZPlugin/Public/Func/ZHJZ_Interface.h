// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ZHJZ_Interface.generated.h"

/**
 * 
 */
UINTERFACE(Blueprintable)
class ZHJZPLUGIN_API UZHJZ_Interface :  public UInterface
{
	GENERATED_BODY()
};

class IZHJZ_Interface
{
	GENERATED_BODY()
public:

	//鼠标停悬
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mouse")
		void MouseOverlay() ;
	
	//鼠标离开
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mouse")
		void MouseLeave() ;

	//鼠标点击
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mouse")
		void MouseClick(bool isBool) ;

	//鼠标停悬_Component
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mouse")
		void MouseOverlay_Component(UMeshComponent* TargetComponent) ;
	
	//鼠标离开_Component
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mouse")
		void MouseLeave_Component(UMeshComponent* TargetComponent) ;

	//鼠标点击_Component
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mouse")
		void MouseClick_Component(UMeshComponent* TargetComponent) ;

	//监控转向
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Mouse")
		void ChangeMonitor(uint8 InDirection);


	//停止播放器
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Media")
		void StopMedia();
};
