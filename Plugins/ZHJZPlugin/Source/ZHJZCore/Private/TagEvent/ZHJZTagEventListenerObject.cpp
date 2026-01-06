#include "TagEvent/ZHJZTagEventListenerObject.h"

void UZHJZTagEventListenerObject::Dispatch(const FZHJZTagEventPayload& Payload)
{
	OnEvent.Broadcast(Payload);
}