#include "Actor/SplineMeshGenerator.h"


ASplineMeshGenerator::ASplineMeshGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASplineMeshGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASplineMeshGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASplineMeshGenerator::GenerateSplineMesh()
{
}

