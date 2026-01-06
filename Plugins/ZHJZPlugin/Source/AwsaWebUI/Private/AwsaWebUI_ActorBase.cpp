#include "AwsaWebUI_ActorBase.h"

#include "AwsaWebUI_GameplayTag.h"
#include "AwsaWebUI_Subsystem.h"


AAwsaWebUI_ActorBase::AAwsaWebUI_ActorBase()
{
	PrimaryActorTick.bCanEverTick = true;
	WebTagContainer.AddTag(AwsaWebUI_Actor);
}

void AAwsaWebUI_ActorBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>())
			{
				Subsystem->RegisterListener(this);
			}
		}
	}
}

void AAwsaWebUI_ActorBase::BeginDestroy()
{
	
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>())
			{
				Subsystem->UnregisterListener(this);
			}
		}
	}
	Super::BeginDestroy();
}

void AAwsaWebUI_ActorBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(WebTagContainer);
}

void AAwsaWebUI_ActorBase::ReceiveFunction_Implementation(const FString& Name, const FString& Message)
{
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("ReceiveFunction is not implemented.Message:::%s"), *Message));
}

void AAwsaWebUI_ActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
