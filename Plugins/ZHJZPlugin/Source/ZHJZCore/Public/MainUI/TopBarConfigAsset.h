#pragma once

#include "CoreMinimal.h"
#include "BarConfigAssetBase.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "TopBarConfigAsset.generated.h"

class UTopBarButtonWidgetBase;

UENUM(BlueprintType)
enum class ETopBarSide : uint8
{
	Left  UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
};

USTRUCT(BlueprintType)
struct FTopBarButtonConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	TSoftClassPtr<UTopBarButtonWidgetBase> ButtonWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	FText ButtonName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	FGameplayTag ButtonTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	ETopBarSide Side = ETopBarSide::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	int32 Index = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	float Offset = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	FString JsonRaw;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	FGameplayTag BottonBarStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	FGuid Id = FGuid::NewGuid();
};


UCLASS(BlueprintType)
class ZHJZCORE_API UTopBarConfigAsset : public UBarConfigAssetBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TopBar")
	TArray<FTopBarButtonConfig> TopButtons;
};