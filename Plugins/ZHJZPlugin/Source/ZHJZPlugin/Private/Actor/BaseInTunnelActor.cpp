#include "Actor/BaseInTunnelActor.h"

#include "AutoParse.h"


ABaseInTunnelActor::ABaseInTunnelActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseInTunnelActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseInTunnelActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ABaseInTunnelActor::MouseClick_Implementation(bool isBool)
{
	IZHJZ_Interface::MouseClick_Implementation(isBool);
	OnPoiClicked();
}

void ABaseInTunnelActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseInTunnelActor::SetInTunnelManger(AInTunnelManager* BelongInTunnelManager)
{
	if (BelongInTunnelManager)
	{
		InTunnelManager = BelongInTunnelManager;
	}
	else
	{

		UAutoParse::PrintLog_GameThread(TEXT("InTunnelManager is nullptr"));
		
	}
}

