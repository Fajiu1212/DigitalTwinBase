#pragma once

#include "CoreMinimal.h"
#include "ZHJZWebUIBroadcastTypes.generated.h"

/**
 * - EventId: 用于区分消息类型
 * - RawJson: 原始 JSON
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FZHJZOnWebUIRawEvent,
	FName, EventId,
	const FString&, RawJson
);
