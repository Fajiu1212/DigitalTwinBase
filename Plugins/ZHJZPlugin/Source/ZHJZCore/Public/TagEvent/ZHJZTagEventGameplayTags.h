#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace ZHJZTagEventTags
{
	// 根前缀，方便 prefix
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagEvent_OpenRoot);

	// 示例：UI/Menu
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagEvent_Menu);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagEvent_Menu_Open);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TagEvent_Menu_Close);
}