#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AwsaPlayerController.generated.h"

UCLASS()
class ZHJZPLUGIN_API AAwsaPlayerController : public APlayerController
{
	GENERATED_BODY()
	AAwsaPlayerController(const class FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable, Category = "Input")
	bool GetMousePositionInWorld(FVector& OutWorldPosition);
};
