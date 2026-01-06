#include "Actor/InTunnelManager.h"

#include "Actor/BaseInTunnelActor.h"


AInTunnelManager::AInTunnelManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInTunnelManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AInTunnelManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AInTunnelManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AInTunnelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FTunnelSplineInfo AInTunnelManager::GetTunnelInfo(const FString& TargetTunnelName)
{
	FTunnelSplineInfo SplineInfo = *TunnelSplineMap.Find(TargetTunnelName);
	return SplineInfo;
}

ABaseInTunnelActor* AInTunnelManager::SpawnTunnelActor(const FInTunnelInfo& Info)
{
	if (InTunnelVehicleClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		if (ABaseInTunnelActor* NewActor = GetWorld()->SpawnActor<ABaseInTunnelActor>(
			InTunnelVehicleClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
		{
			NewActor->SetInTunnelManger(this);
			//NewActor->UpdateInTunnelInfo();
			return NewActor;
		}
	}
	return nullptr;
}

void AInTunnelManager::DestroyTunnelActor()
{
}
