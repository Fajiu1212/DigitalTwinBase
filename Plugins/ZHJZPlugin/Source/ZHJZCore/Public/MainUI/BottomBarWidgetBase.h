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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BottomBar")
	TObjectPtr<UBottomBarConfigAsset> Config;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BottomBar")
	FGameplayTag DefaultBottomBarStyle;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> BottomBarBox;
	
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;

	
private:
	UFUNCTION()
	void RebuildFromConfig(FGameplayTag TargetStyle);
	void ClearBoxes();

	void AddButtonToBox(
		UBottomBarButtonWidgetBase* ButtonWidget,
		const struct FBottomBarButtonConfig& Cfg
	);

	static void SortGroup(TArray<const struct FBottomBarButtonConfig*>& InOut);
};
