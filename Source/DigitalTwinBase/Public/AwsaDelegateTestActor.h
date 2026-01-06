#pragma once

#include "CoreMinimal.h"
#include "AwsaDelegate.h"
#include "GameFramework/Actor.h"
#include "AwsaDelegateTestActor.generated.h"


UCLASS()
class DIGITALTWINBASE_API AAwsaDelegateTestActor : public AActor
{
	GENERATED_BODY()

public:
	AAwsaDelegateTestActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	FTestDelegate TestDelegate; 
};
