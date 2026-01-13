#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayTagContainer.h"
#include "TagEvent/ZHJZTagEventTypes.h"
#include "UZHJZTagEventAsync.generated.h"

class UZHJZTagEventBusSubsystem;
class UZHJZTagEventListenerObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZHJZOnFilteredGlobalTagEventAsync, const FZHJZTagEventPayload&, Payload);

UCLASS()
class ZHJZCORE_API UZHJZTagEventAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="ZHJZ|TagEvent")
	FZHJZOnFilteredGlobalTagEventAsync OnEvent;

	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="ZHJZ|TagEvent")
	static UZHJZTagEventAsync* ListenFilteredGlobalTagEvent(
		UObject* WorldContextObject,
		FGameplayTag TagOrPrefix,
		bool bPrefixMatch
	);

	UFUNCTION(BlueprintCallable, Category="ZHJZ|TagEvent")
	void Stop();

	virtual void Activate() override;

protected:
	virtual void BeginDestroy() override;

private:
	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject;

	UPROPERTY()
	FGameplayTag FilterTagOrPrefix;

	UPROPERTY()
	bool bPrefixMatch = false;

	UPROPERTY()
	TObjectPtr<UZHJZTagEventListenerObject> Listener;

	UFUNCTION()
	void HandleListenerEvent(const FZHJZTagEventPayload& Payload);

	UZHJZTagEventBusSubsystem* GetBus() const;
	bool PassFilter(const FGameplayTag& EventTag) const;
};