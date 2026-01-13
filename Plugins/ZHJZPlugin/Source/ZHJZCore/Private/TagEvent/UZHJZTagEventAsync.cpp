#include "TagEvent/UZHJZTagEventAsync.h"

#include "TagEvent/ZHJZTagEventBusSubsystem.h"
#include "TagEvent/ZHJZTagEventListenerObject.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UZHJZTagEventAsync* UZHJZTagEventAsync::ListenFilteredGlobalTagEvent(
	UObject* WorldContextObject,
	FGameplayTag TagOrPrefix,
	bool bInPrefixMatch)
{
	UZHJZTagEventAsync* Node = NewObject<UZHJZTagEventAsync>();
	Node->WorldContextObject = WorldContextObject;
	Node->FilterTagOrPrefix = TagOrPrefix;
	Node->bPrefixMatch = bInPrefixMatch;
	return Node;
}

UZHJZTagEventBusSubsystem* UZHJZTagEventAsync::GetBus() const
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UZHJZTagEventBusSubsystem>() : nullptr;
}

bool UZHJZTagEventAsync::PassFilter(const FGameplayTag& EventTag) const
{
	if (!FilterTagOrPrefix.IsValid())
	{
		return false;
	}

	return bPrefixMatch ? EventTag.MatchesTag(FilterTagOrPrefix) : (EventTag == FilterTagOrPrefix);
}

void UZHJZTagEventAsync::Activate()
{
	if (!WorldContextObject || !FilterTagOrPrefix.IsValid())
	{
		SetReadyToDestroy();
		return;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		SetReadyToDestroy();
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		SetReadyToDestroy();
		return;
	}

	RegisterWithGameInstance(GI);

	if (UZHJZTagEventBusSubsystem* Bus = GI->GetSubsystem<UZHJZTagEventBusSubsystem>())
	{
		Listener = Bus->GetGlobalListener();
		if (!Listener)
		{
			SetReadyToDestroy();
			return;
		}

		Listener->OnEvent.AddUniqueDynamic(this, &UZHJZTagEventAsync::HandleListenerEvent);
	}
	else
	{
		SetReadyToDestroy();
	}
}

void UZHJZTagEventAsync::HandleListenerEvent(const FZHJZTagEventPayload& Payload)
{
	if (PassFilter(Payload.EventTag))
	{
		OnEvent.Broadcast(Payload);
	}
}

void UZHJZTagEventAsync::Stop()
{
	if (Listener)
	{
		Listener->OnEvent.RemoveDynamic(this, &UZHJZTagEventAsync::HandleListenerEvent);
		Listener = nullptr;
	}

	SetReadyToDestroy();
}

void UZHJZTagEventAsync::BeginDestroy()
{
	if (Listener)
	{
		Listener->OnEvent.RemoveDynamic(this, &UZHJZTagEventAsync::HandleListenerEvent);
		Listener = nullptr;
	}

	Super::BeginDestroy();
}