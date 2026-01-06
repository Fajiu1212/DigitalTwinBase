#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"

#include "TagEvent/ZHJZTagEventTypes.h"
#include "TagEvent/ZHJZTagEventListenerObject.h"

#include "ZHJZTagEventBusSubsystem.generated.h"

UCLASS()
class ZHJZCORE_API UZHJZTagEventBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 全局唯一监听器 */
	UFUNCTION(BlueprintCallable, Category="ZHJZ|TagEvent")
	UZHJZTagEventListenerObject* GetGlobalListener() const { return GlobalListener; }

	/** 广播 蓝图 */
	UFUNCTION(BlueprintCallable, Category="ZHJZ|TagEvent")
	void BroadcastTagEvent(FGameplayTag EventTag, const FString& RawJson);

	// ---- Subscribe/Unsubscribe ----
 
	// FZHJZTagEventSubscriptionHandle Subscribe(FGameplayTag TagOrPrefix, bool bPrefixMatch, UObject* Owner, FZHJZTagEventCallback Callback);
	// void Unsubscribe(FZHJZTagEventSubscriptionHandle Handle);
	// void UnsubscribeAllForOwner(UObject* Owner);

private:
	struct FRecord
	{
		int64 Id = 0;
		FGameplayTag TagOrPrefix;
		bool bPrefixMatch = false;
		TWeakObjectPtr<UObject> Owner;
		FZHJZTagEventCallback Callback;
	};

	UPROPERTY()
	TObjectPtr<UZHJZTagEventListenerObject> GlobalListener;

	TArray<FRecord> Records;
	int64 NextId = 1;

	void CleanupInvalid();
};