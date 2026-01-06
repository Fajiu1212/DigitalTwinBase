#pragma once

#include "CoreMinimal.h"

struct FEventGameplayTagListItem
{
	FString Tag;
	FString DevComment;
};

class FZHJZGameplayTagsIniUtils
{
public:
	static bool LoadEventTagsFromDataTable(TArray<FEventGameplayTagListItem>& OutItems, FString* OutError = nullptr);
	static bool SaveEventTagsToDataTable(const TArray<FEventGameplayTagListItem>& Items, FString* OutError = nullptr);

	static void RefreshGameplayTagTreeInEditor();

	static bool IsValidGameplayTagString(const FString& TagStr);
	static bool IsEventTag(const FString& TagStr);
};