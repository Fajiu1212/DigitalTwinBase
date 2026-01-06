#pragma once

#include "CoreMinimal.h"
#include "Actor/BeamGeneratorSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "BeamWidgetBase.generated.h"

UCLASS()
class ZHJZPLUGIN_API UBeamWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(BlueprintReadWrite, Category="Beam|BeamPoiWidget")
	FBeamStruct BeamStruct;

public:
	UFUNCTION(BlueprintNativeEvent, Category="Beam|BeamPoiWidget")
	void SetBeamStruct(const FBeamStruct& TargetBeamStruct);
};
