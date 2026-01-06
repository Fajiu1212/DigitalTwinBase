// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PositionDisplayUI.generated.h"

/**
 * 
 */
UCLASS()
class ZHJZPLUGIN_API UPositionDisplayUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Position Display")
	void SetPositionText(const FVector& Position);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PositionText;
	
};
