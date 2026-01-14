#pragma once
#include "CoreMinimal.h"
class UUserWidget;

namespace ZHJZWidgetWorldUtils
{
	static inline UWorld* ResolveWorld(UUserWidget* Widget)
	{
		if (!Widget) return nullptr;
		if (UWorld* World = Widget->GetWorld()) return World;
		if (UWorld* OuterWorld = Widget->GetTypedOuter<UWorld>()) return OuterWorld;
		if (UObject* Outer = Widget->GetOuter()) return Outer->GetWorld();
		return nullptr;
	}
}