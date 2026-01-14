#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FEventGameplayTagListItem;
class IDetailsView;
class SEditableTextBox;
class UBarConfigAssetBase;

class SZHJZTagManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZHJZTagManagerWidget)
		{
		}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	using FItem = TSharedPtr<FEventGameplayTagListItem>;

private:
	// List
	TArray<FItem> Items;
	TSharedPtr<SListView<FItem>> ListView;

	TSharedPtr<SEditableTextBox> NewTagTextBox;
	TSharedPtr<SEditableTextBox> NewCommentTextBox;

	// Actions
	FReply OnReload();
	FReply OnAdd();
	FReply OnRemoveSelected();
	FReply OnApply();

	void ReloadFromDatatable();

	TSharedRef<ITableRow> OnGenerateRow(FItem Item, const TSharedRef<STableViewBase>& OwnerTable);

	FItem GetSelectedItem() const;

	// DataAssets part
	TWeakObjectPtr<UBarConfigAssetBase> EditingBarAsset;
	TSharedPtr<IDetailsView> BarDetailsView;

	void SetEditingBarAsset(UBarConfigAssetBase* NewAsset);
	FReply OnSaveBarAsset();

public:
	bool TryRenameItemTag(const FItem& Item, const FString& NewTagStr, FString& OutError);

	void NotifyUser(const FString& Msg, bool bSuccess) const;
};
