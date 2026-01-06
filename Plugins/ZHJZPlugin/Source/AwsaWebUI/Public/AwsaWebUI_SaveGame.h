// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AwsaWebUI_SaveGame.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AWSAWEBUI_API UAwsaWebUI_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	FString Version;
};
