#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineMeshGenerator.generated.h"

UCLASS()
class ZHJZPLUGIN_API ASplineMeshGenerator : public AActor
{
	GENERATED_BODY()

public:
	ASplineMeshGenerator();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Spline Generation")
	void GenerateSplineMesh();
};
