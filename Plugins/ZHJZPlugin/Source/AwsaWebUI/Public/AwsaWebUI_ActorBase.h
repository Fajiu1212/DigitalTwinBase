#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "AwsaWebUI_Interface.h"
#include "GameFramework/Actor.h"
#include "AwsaWebUI_ActorBase.generated.h"

UCLASS(BlueprintType)
class AWSAWEBUI_API AAwsaWebUI_ActorBase : public AActor, public IAwsaWebUI_Interface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	AAwsaWebUI_ActorBase();

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
