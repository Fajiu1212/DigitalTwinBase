#pragma once

#include "CoreMinimal.h"
#include "BarConfigAssetBase.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MenuEntryWidgetBase.h"

#include "BottomBarConfigAsset.generated.h"


class UBottomBarButtonWidgetBase;

// USTRUCT(BlueprintType)
// struct FMenuEntryButtonConfig
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
// 	TSubclassOf<UMenuEntryWidgetBase> WidgetClass;
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
// 	FGameplayTag ButtonTag;
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
// 	FText ButtonName;
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
// 	FString JsonRaw = TEXT("{}");
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
// 	int32 Index = 0;
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
// 	FGuid Id = FGuid::NewGuid();
// };

USTRUCT(BlueprintType)
struct FBottomBarButtonConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	TSoftClassPtr<UBottomBarButtonWidgetBase> ButtonWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	FGameplayTag ButtonTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	FText ButtonName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	UTexture2D* Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	TArray<FMenuEntryConfig> BelongMenuEntryButtonConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	int32 Index = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	FString JsonRaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	FGuid Id = FGuid::NewGuid();
};

USTRUCT(BlueprintType)
struct FBottomBarConfigStruct
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	TArray<FBottomBarButtonConfig> BottomBarButtons;
};

UCLASS(BlueprintType)
class ZHJZCORE_API UBottomBarConfigAsset : public UBarConfigAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BottomBar")
	TMap<FGameplayTag, FBottomBarConfigStruct> BottomBarConfig;
};
