#include "TagTools/ZHJZGameplayTagsIniUtils.h"

#include "GameplayTagsManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "EditorAssetLibrary.h"

static const TCHAR* ZHJZEventRootPrefix = TEXT("EventTag");

static UDataTable* GetZHJZEventTagsTable(FString* OutError)
{
	const TSoftObjectPtr<UDataTable> TableRef(
		FSoftObjectPath(TEXT("/ZHJZPlugin/Data/DT_ZHJZEventTags.DT_ZHJZEventTags"))
	);

	UDataTable* Table = TableRef.LoadSynchronous();
	if (!Table)
	{
		if (OutError)
		{
			*OutError = TEXT("Failed to load DT_ZHJZEventTags from /ZHJZPlugin/Data.");
		}
		return nullptr;
	}
	return Table;
}

bool FZHJZGameplayTagsIniUtils::IsEventTag(const FString& TagStr)
{
	return TagStr.Equals(ZHJZEventRootPrefix) || TagStr.StartsWith(FString(ZHJZEventRootPrefix) + TEXT("."));
}

bool FZHJZGameplayTagsIniUtils::IsValidGameplayTagString(const FString& TagStr)
{
	if (TagStr.IsEmpty() || TagStr.Contains(TEXT(" ")))
	{
		return false;
	}

	for (TCHAR C : TagStr)
	{
		const bool bOk =
			(C >= 'a' && C <= 'z') ||
			(C >= 'A' && C <= 'Z') ||
			(C >= '0' && C <= '9') ||
			(C == '_') ||
			(C == '.');

		if (!bOk)
		{
			return false;
		}
	}

	return true;
}

bool FZHJZGameplayTagsIniUtils::LoadEventTagsFromDataTable(TArray<FEventGameplayTagListItem>& OutItems,
                                                           FString* OutError)
{
	OutItems.Reset();

	UDataTable* Table = GetZHJZEventTagsTable(OutError);
	if (!Table) return false;

	if (!Table->GetRowStruct() || Table->GetRowStruct()->GetFName() != FName(TEXT("GameplayTagTableRow")))
	{
		if (OutError)
		{
			*OutError = TEXT("Invalid datatable row.");
		}
		return false;
	}

	TArray<FName> RowNames = Table->GetRowNames();
	RowNames.Sort(FNameLexicalLess());

	TSet<FString> Seen;

	for (const FName RowName : RowNames)
	{
		const FGameplayTagTableRow* Row = Table->FindRow<FGameplayTagTableRow>(RowName, TEXT("ZHJZEventTagsLoad"));
		if (!Row)
		{
			continue;
		}

		const FString TagStr = Row->Tag.ToString();

		if (!IsEventTag(TagStr))
		{
			continue;
		}

		if (Seen.Contains(TagStr))
		{
			continue;
		}
		Seen.Add(TagStr);

		FEventGameplayTagListItem Item;
		Item.Tag = TagStr;
		Item.DevComment = Row->DevComment;
		OutItems.Add(MoveTemp(Item));
	}

	OutItems.Sort([](const FEventGameplayTagListItem& A, const FEventGameplayTagListItem& B)
	{
		return A.Tag < B.Tag;
	});

	return true;
}

bool FZHJZGameplayTagsIniUtils::SaveEventTagsToDataTable(const TArray<FEventGameplayTagListItem>& Items,
                                                         FString* OutError)
{
	UDataTable* Table = GetZHJZEventTagsTable(OutError);
	if (!Table)
	{
		return false;
	}

	if (!Table->GetRowStruct() || Table->GetRowStruct()->GetFName() != FName(TEXT("GameplayTagTableRow")))
	{
		if (OutError)
		{
			*OutError = TEXT("Invalid datatable row.");
		}
		return false;
	}

	// 归一化、校验、去重
	TMap<FString, FString> TagToComment;

	auto AddOrUpdate = [&](const FString& TagStrIn, const FString& CommentIn) -> bool
	{
		const FString TagStr = TagStrIn.TrimStartAndEnd();
		if (TagStr.IsEmpty())
		{
			return true;
		}
		if (!IsValidGameplayTagString(TagStr))
		{
			if (OutError)
			{
				*OutError = FString::Printf(TEXT("Invalid tag string: %s"), *TagStr);
			}
			return false;
		}
		if (!IsEventTag(TagStr))
		{
			if (OutError)
			{
				*OutError = FString::Printf(TEXT("Tag must be under %s: %s"), ZHJZEventRootPrefix, *TagStr);
			}
			return false;
		}

		TagToComment.FindOrAdd(TagStr) = CommentIn;
		return true;
	};

	// Root 保障
	if (!AddOrUpdate(ZHJZEventRootPrefix, TEXT("Root for ZHJZ event tags")))
	{
		return false;
	}

	for (const FEventGameplayTagListItem& Item : Items)
	{
		if (!AddOrUpdate(Item.Tag, Item.DevComment))
		{
			return false;
		}
	}

	// Delete
	{
		const TArray<FName> ExistingRowNames = Table->GetRowNames();

		for (const FName RowName : ExistingRowNames)
		{
			const FGameplayTagTableRow* Row = Table->FindRow<FGameplayTagTableRow>(RowName, TEXT("ZHJZEventTagsSave"));
			if (!Row)
			{
				continue;
			}

			const FString ExistingTagStr = Row->Tag.ToString();
			if (!IsEventTag(ExistingTagStr))
			{
				continue;
			}

			if (!TagToComment.Contains(ExistingTagStr))
			{
				Table->RemoveRow(RowName);
			}
		}
	}
	
	for (const TPair<FString, FString>& Pair : TagToComment)
	{
		const FString& TagStr = Pair.Key;
		const FString& Comment = Pair.Value;

		FString RowNameStr = TagStr;
		RowNameStr.ReplaceInline(TEXT("."), TEXT("_"));
		RowNameStr.ReplaceInline(TEXT(":"), TEXT("_"));
		const FName RowName(*RowNameStr);

		FGameplayTagTableRow NewRow;
		NewRow.Tag = FName(*TagStr);
		NewRow.DevComment = Comment;

		Table->RemoveRow(RowName);
		Table->AddRow(RowName, NewRow);
	}
	
	Table->MarkPackageDirty();

	
	const bool bSaved = UEditorAssetLibrary::SaveLoadedAsset(Table, true);
	if (!bSaved)
	{
		if (OutError) *OutError = TEXT("Failed to save DT_ZHJZEventTags (asset save failed).");
		return false;
	}

	UGameplayTagsManager::Get().EditorRefreshGameplayTagTree();
	return true;
}

void FZHJZGameplayTagsIniUtils::RefreshGameplayTagTreeInEditor()
{
	UGameplayTagsManager::Get().EditorRefreshGameplayTagTree();
}
