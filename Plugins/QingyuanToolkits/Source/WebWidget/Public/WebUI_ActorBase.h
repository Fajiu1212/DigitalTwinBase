#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "WebUI_Interface.h"
#include "GameFramework/Actor.h"
#include "WebUI_ActorBase.generated.h"

UCLASS(BlueprintType)
class WEBWIDGET_API AWebUI_ActorBase : public AActor, public IWebWidget_Interface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AWebUI_ActorBase();

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

public:
	virtual void Tick(float DeltaTime) override;

	//Gameplay tag
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "WebUI|GameplayTag")
	FGameplayTagContainer WebTagContainer;
	//Receive webui msg
	virtual void ReceiveFunction_Implementation(const FString& Name, const FString& Message) override;
};
