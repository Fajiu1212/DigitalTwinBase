#include "WebUI_ActorBase.h"

#include "WebUI_GameplayTag.h"
#include "WebUI_Subsystem.h"


AWebUI_ActorBase::AWebUI_ActorBase()
{
	PrimaryActorTick.bCanEverTick = true;
	WebTagContainer.AddTag(WebUI_Actor);
}

void AWebUI_ActorBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UWebUI_Subsystem>())
			{
				Subsystem->RegisterListener(this);
			}
		}
	}
}

void AWebUI_ActorBase::BeginDestroy()
{
	
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UWebUI_Subsystem>())
			{
				Subsystem->UnregisterListener(this);
			}
		}
	}
	Super::BeginDestroy();
}

void AWebUI_ActorBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(WebTagContainer);
}

void AWebUI_ActorBase::ReceiveFunction_Implementation(const FString& Name, const FString& Message)
{
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("ReceiveFunction is not implemented.Message:::%s"), *Message));
}

void AWebUI_ActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
