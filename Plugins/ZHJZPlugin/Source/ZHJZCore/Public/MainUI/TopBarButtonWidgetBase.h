#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BarButtonWidgetBase.h"
#include "TopBarConfigAsset.h"

#include "TopBarButtonWidgetBase.generated.h"

UCLASS()
class ZHJZCORE_API UTopBarButtonWidgetBase : public UBarButtonWidgetBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="ZHJZ|TopBar")
	void ApplyRowData(const FTopBarButtonConfig& InRow);

	UFUNCTION(BlueprintImplementableEvent, Category="ZHJZ|TopBar")
	void BP_OnRowDataApplied();
	
	UPROPERTY(BlueprintReadWrite, Category="ZHJZ|TopBar")
	FTopBarButtonConfig RowData;

	UFUNCTION(BlueprintCallable, Category="ZHJZ|TopBar")
	void HandleClicked();
	
	UFUNCTION(BlueprintPure, Category="ZHJZ|TopBar")
	FGameplayTag GetButtonTag() const;
};
