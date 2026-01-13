#pragma once

#include "CoreMinimal.h"
#include "MenuEntryConfigAsset.h"
#include "Blueprint/UserWidget.h"
#include "MenuEntryWidgetBase.generated.h"

UCLASS()
class ZHJZCORE_API UMenuEntryWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="ZHJZ|MenuEntry")
	void ApplyRowData(const FMenuEntryConfig& InRow);

	UFUNCTION(BlueprintImplementableEvent, Category="ZHJZ|MenuEntry")
	void BP_OnRowDataApplied();
	
	UPROPERTY(BlueprintReadWrite, Category="ZHJZ|MenuEntry")
	FMenuEntryConfig RowData;

	UFUNCTION(BlueprintCallable, Category="ZHJZ|MenuEntry")
	void HandleClicked();

	UFUNCTION(BlueprintPure, Category="ZHJZ|MenuEntry")
	FGameplayTag GetLabelTag() const;
};
