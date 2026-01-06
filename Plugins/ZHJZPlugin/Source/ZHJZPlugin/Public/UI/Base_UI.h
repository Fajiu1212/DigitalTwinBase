// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Base_UI.generated.h"

/**
 * 
 */
UCLASS()
class ZHJZPLUGIN_API UBase_UI : public UUserWidget
{
	GENERATED_BODY()


protected:

	virtual void NativeConstruct() override;

	//为Button/CheckBox点击事件添加音效
	void AddClickedSound();


	//button点击播放
	UFUNCTION()
		void PlayBtnOnClickSound();

	//CheckBox点击播放
	UFUNCTION()
		void PlayCheckBoxOnClickSound();

public:

	//音效开关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Setting")
		bool  IsPlaySound;

	//音效文件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Setting")
		USoundWave* Sound;
	
};
