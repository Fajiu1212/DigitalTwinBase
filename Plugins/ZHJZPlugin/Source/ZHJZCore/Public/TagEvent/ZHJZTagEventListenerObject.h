#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TagEvent/ZHJZTagEventTypes.h"
#include "ZHJZTagEventListenerObject.generated.h"

/**
 * 全局 Tag 事件监听器对象：
 * - 由 UZHJZTagEventBusSubsystem 在 Initialize 时创建并持有
 * - 蓝图对 OnEvent 进行 Assign/Bind 来接收所有 Tag 事件
 *
 * 注意：它不做过滤；过滤通过 Payload.EventTag 在蓝图里分发（Switch/MatchesTag）。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FZHJZOnTagEvent, const FZHJZTagEventPayload&, Payload);

UCLASS(BlueprintType)
class ZHJZCORE_API UZHJZTagEventListenerObject : public UObject
{
	GENERATED_BODY()

public:
	/** 蓝图可绑定的事件入口 */
	UPROPERTY(BlueprintAssignable, Category="ZHJZ|TagEvent")
	FZHJZOnTagEvent OnEvent;

	/** Subsystem 内部调用：把事件推给蓝图 */
	void Dispatch(const FZHJZTagEventPayload& Payload);
};