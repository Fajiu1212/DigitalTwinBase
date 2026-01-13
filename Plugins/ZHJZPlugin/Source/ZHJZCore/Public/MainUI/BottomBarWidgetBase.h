#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "BottomBarWidgetBase.generated.h"

class UBottomBarButtonWidgetBase;
class UHorizontalBox;
class UBottomBarConfigAsset;

UCLASS()
class ZHJZCORE_API UBottomBarWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TopBar")
	TObjectPtr<UBottomBarConfigAsset> Config;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TopBar")
	FGameplayTag DefaultBottomBarStyle;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> BottomBarBox;
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;

	
private:
	void RebuildFromConfig(bool bIsDesignTime, FGameplayTag TargetStyle);
	void ClearBoxes();

	void AddButtonToBox(
		UBottomBarButtonWidgetBase* ButtonWidget,
		const struct FBottomBarButtonConfig& Cfg
	);

	static void SortGroup(TArray<const struct FBottomBarButtonConfig*>& InOut);
};
