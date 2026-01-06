#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "InTunnelSpline.generated.h"

UCLASS()
class ZHJZPLUGIN_API AInTunnelSpline : public AActor
{
	GENERATED_BODY()

public:
	AInTunnelSpline();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite,Category = "InTunnelSpline")
	USplineComponent* SplineComponent;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="InTunnelSpline")
	USplineComponent* GetSplineComponent();
};
