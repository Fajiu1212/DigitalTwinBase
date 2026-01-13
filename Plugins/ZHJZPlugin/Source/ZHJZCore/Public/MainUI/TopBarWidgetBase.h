#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "TopBarWidgetBase.generated.h"

class UHorizontalBox;
class UTopBarButtonWidgetBase;
class UTopBarConfigAsset;

UCLASS(Abstract, BlueprintType, Blueprintable)
class ZHJZCORE_API UTopBarWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TopBar")
	TObjectPtr<UTopBarConfigAsset> Config;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> LeftBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> RightBox;

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SynchronizeProperties() override;

private:
	void RebuildFromConfig(bool bIsDesignTime);
	void ClearBoxes();

	void AddButtonToBox(
		UHorizontalBox* TargetBox,
		UTopBarButtonWidgetBase* ButtonWidget,
		const struct FTopBarButtonConfig& Cfg
	);

	static void SortGroup(TArray<const struct FTopBarButtonConfig*>& InOut);
};