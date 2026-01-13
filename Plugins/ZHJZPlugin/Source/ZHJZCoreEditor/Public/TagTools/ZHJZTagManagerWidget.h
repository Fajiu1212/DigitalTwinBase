#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

struct FEventGameplayTagListItem;

class SZHJZTagManagerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZHJZTagManagerWidget)
		{
		}

	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);

private:
	using FItem = TSharedPtr<FEventGameplayTagListItem>;

	TArray<FItem> Items;
	TSharedPtr<SListView<FItem>> ListView;

	TSharedPtr<class SEditableTextBox> NewTagTextBox;
	TSharedPtr<class SEditableTextBox> NewCommentTextBox;

	FReply OnReload();
	FReply OnAdd();
	FReply OnRemoveSelected();
	FReply OnApply();

	void ReloadFromDatatable();
	TSharedRef<class ITableRow> OnGenerateRow(FItem Item, const TSharedRef<class STableViewBase>& OwnerTable);

	FItem GetSelectedItem() const;
	
	// DataAssets part
	TWeakObjectPtr<class UBarConfigAssetBase> EditingBarAsset;
	TSharedPtr<class IDetailsView> BarDetailsView;

	void SetEditingBarAsset(class UBarConfigAssetBase* NewAsset);
	FReply OnSaveBarAsset();
};
