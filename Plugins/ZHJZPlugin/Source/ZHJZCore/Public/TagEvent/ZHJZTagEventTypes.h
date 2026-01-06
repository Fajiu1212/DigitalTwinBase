#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZHJZTagEventTypes.generated.h"

/** 通用 Payload：Tag + RawJson */
USTRUCT(BlueprintType)
struct FZHJZTagEventPayload
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="ZHJZ|TagEvent")
	FGameplayTag EventTag;

	UPROPERTY(BlueprintReadOnly, Category="ZHJZ|TagEvent")
	FString RawJson;
};

/** 取消订阅Handle */
USTRUCT(BlueprintType)
struct FZHJZTagEventSubscriptionHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="ZHJZ|TagEvent")
	int64 Id = 0;

	bool IsValid() const { return Id != 0; }
};

/** Async回调 */
DECLARE_DELEGATE_OneParam(FZHJZTagEventCallback, const FZHJZTagEventPayload&);