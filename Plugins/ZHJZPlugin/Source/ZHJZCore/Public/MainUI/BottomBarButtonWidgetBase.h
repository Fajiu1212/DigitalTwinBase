#pragma once

#include "CoreMinimal.h"
#include "BarButtonWidgetBase.h"
#include "BottomBarConfigAsset.h"
#include "TopBarConfigAsset.h"
#include "Components/VerticalBox.h"
#include "BottomBarButtonWidgetBase.generated.h"

UCLASS()
class ZHJZCORE_API UBottomBarButtonWidgetBase : public UBarButtonWidgetBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ZHJZ|BottomBar", meta=(BindWidget))
	TObjectPtr<UVerticalBox> VerticalBox_MenuEntryBox;
	
	UFUNCTION(BlueprintCallable, Category="ZHJZ|BottomBar")
	void ApplyRowData(const FBottomBarButtonConfig& InRow);
	
	UFUNCTION(BlueprintImplementableEvent, Category="ZHJZ|BottomBar")
	void BP_OnRowDataApplied();
	
	UPROPERTY(BlueprintReadWrite, Category="ZHJZ|BottomBar")
	FBottomBarButtonConfig RowData;
	
	UFUNCTION(BlueprintCallable, Category="ZHJZ|BottomBar")
	void HandleClicked();
	
	UFUNCTION(BlueprintPure, Category="ZHJZ|BottomBar")
	FGameplayTag GetButtonTag() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="ZHJZ|BottomBar")
	void AddMenuEntry();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="ZHJZ|BottomBar")
	void AddButtonToBox(UMenuEntryWidgetBase* ButtonWidget);
};
