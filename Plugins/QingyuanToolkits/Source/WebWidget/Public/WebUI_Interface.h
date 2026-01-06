#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WebUI_Interface.generated.h"

UINTERFACE(Blueprintable)
class UWebWidget_Interface : public UInterface
{
	GENERATED_BODY()
};
class WEBWIDGET_API IWebWidget_Interface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "WebUI|Interface")
	void ReceiveFunction(const FString& Name,const FString& Message);
	virtual void ReceiveFunction_Implementation(const FString& Name,const FString& Message);
};
