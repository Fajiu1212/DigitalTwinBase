#include "Actor/TunnelBase.h"


ATunnelBase::ATunnelBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATunnelBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATunnelBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

