#include "Actor/InTunnelSpline.h"


AInTunnelSpline::AInTunnelSpline()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(RootComponent);
}

void AInTunnelSpline::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInTunnelSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

USplineComponent* AInTunnelSpline::GetSplineComponent()
{
	return SplineComponent;
}

