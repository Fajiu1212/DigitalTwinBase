#include "TagEvent/ZHJZTagEventBusSubsystem.h"

void UZHJZTagEventBusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 创建全局唯一 Listener（Outer=Subsystem），生命周期跟随 GameInstance
	GlobalListener = NewObject<UZHJZTagEventListenerObject>(this);
}

void UZHJZTagEventBusSubsystem::Deinitialize()
{
	// 释放引用即可（Outer 链会一起清理）
	GlobalListener = nullptr;
	Records.Reset();

	Super::Deinitialize();
}

void UZHJZTagEventBusSubsystem::CleanupInvalid()
{
	Records.RemoveAll([](const FRecord& R)
	{
		return R.Id == 0
			|| !R.Owner.IsValid()
			|| !R.TagOrPrefix.IsValid()
			|| !R.Callback.IsBound();
	});
}

// FZHJZTagEventSubscriptionHandle UZHJZTagEventBusSubsystem::Subscribe(
// 	FGameplayTag TagOrPrefix,
// 	bool bPrefixMatch,
// 	UObject* Owner,
// 	FZHJZTagEventCallback Callback)
// {
// 	FZHJZTagEventSubscriptionHandle Handle;
//
// 	if (!Owner || !TagOrPrefix.IsValid() || !Callback.IsBound())
// 	{
// 		return Handle;
// 	}
//
// 	CleanupInvalid();
//
// 	FRecord Rec;
// 	Rec.Id = NextId++;
// 	Rec.TagOrPrefix = TagOrPrefix;
// 	Rec.bPrefixMatch = bPrefixMatch;
// 	Rec.Owner = Owner;
// 	Rec.Callback = MoveTemp(Callback);
//
// 	Records.Add(MoveTemp(Rec));
// 	Handle.Id = Records.Last().Id;
// 	return Handle;
// }

// void UZHJZTagEventBusSubsystem::Unsubscribe(FZHJZTagEventSubscriptionHandle Handle)
// {
// 	if (!Handle.IsValid())
// 	{
// 		return;
// 	}
//
// 	Records.RemoveAll([&](const FRecord& R)
// 	{
// 		return R.Id == Handle.Id;
// 	});
// }
//
// void UZHJZTagEventBusSubsystem::UnsubscribeAllForOwner(UObject* Owner)
// {
// 	if (!Owner)
// 	{
// 		return;
// 	}
//
// 	Records.RemoveAll([&](const FRecord& R)
// 	{
// 		return R.Owner.Get() == Owner;
// 	});
// }

void UZHJZTagEventBusSubsystem::BroadcastTagEvent(FGameplayTag EventTag, const FString& RawJson)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	CleanupInvalid();

	FZHJZTagEventPayload Payload;
	Payload.EventTag = EventTag;
	Payload.RawJson = RawJson;

	// 1) 先推给全局蓝图监听器
	if (GlobalListener)
	{
		GlobalListener->Dispatch(Payload);
	}

	// 2) 再分发给 C++ 订阅者（可选：顺序你也可以反过来）
	const TArray<FRecord> Snapshot = Records;
	for (const FRecord& R : Snapshot)
	{
		UObject* OwnerObj = R.Owner.Get();
		if (!OwnerObj) continue;

		const bool bMatch = R.bPrefixMatch ? EventTag.MatchesTag(R.TagOrPrefix) : (EventTag == R.TagOrPrefix);
		if (!bMatch) continue;

		if (R.Callback.IsBound())
		{
			R.Callback.Execute(Payload);
		}
	}
}