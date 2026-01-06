#pragma once

#include "CoreMinimal.h"
#include "UI/Base_UI.h"
#include "Engine/Datatable.h"
#include "StructDefine.generated.h"

//当前UI信息
USTRUCT(BlueprintType)
struct  FCurrentUIMes
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(BlueprintReadOnly)
		FString  UIName;

	UPROPERTY(BlueprintReadOnly)
		UBase_UI* CurrentWidget;

	FCurrentUIMes()
	{
		UIName = "";
		CurrentWidget = nullptr;

	}
};