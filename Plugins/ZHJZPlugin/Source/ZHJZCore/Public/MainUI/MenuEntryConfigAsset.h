#pragma once

#include "CoreMinimal.h"
#include "BarConfigAssetBase.h"
#include "GameplayTagContainer.h"
#include "MenuEntryConfigAsset.generated.h"

class UMenuEntryWidgetBase;

USTRUCT(BlueprintType)
struct FMenuEntryConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuEntry")
	TSoftClassPtr<UMenuEntryWidgetBase> MenuEntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuEntry")
	FText  Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuEntry")
	FGameplayTag LabelTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuEntry")
	int32 Index = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuEntry")
	FString JsonRaw;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuEntry")
	FGuid Id = FGuid::NewGuid();
};

UCLASS()
class ZHJZCORE_API UMenuEntryConfigAsset : public UBarConfigAssetBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MenuEntry")
	TArray<FMenuEntryConfig> FMenuEntryWidgets;
};
