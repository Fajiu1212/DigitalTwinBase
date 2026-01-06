#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "WebUI_Interface.h"
#include "UObject/Object.h"
#include "WebUI_Manager.generated.h"

UCLASS(Blueprintable)
class WEBWIDGET_API UWebUI_Manager : public UObject, public IWebWidget_Interface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	virtual class UWorld* GetWorld() const override;
	class ULevel* GetLevel() const;
	
	//Gameplay tag
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "WebUI|GameplayTag")
	FGameplayTagContainer WebTagContainer;
	
	//Web Interface
	virtual void ReceiveFunction_Implementation(const FString& Name, const FString& Message) override;

	//Default func
	UFUNCTION(BlueprintCallable, Category = "WebUI|Interface")
	void ExitGame();
};
