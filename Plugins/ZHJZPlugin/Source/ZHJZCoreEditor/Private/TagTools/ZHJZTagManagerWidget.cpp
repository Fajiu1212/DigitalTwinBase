#include "TagTools/ZHJZTagManagerWidget.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Styling/AppStyle.h"
#include "TagTools/ZHJZGameplayTagsIniUtils.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

// DataAsset editor
#include "EditorAssetLibrary.h"
#include "IDetailsView.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "MainUI/TopBarConfigAsset.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

static void Notify(const FString& Msg, SNotificationItem::ECompletionState State)
{
	FNotificationInfo Info(FText::FromString(Msg));
	Info.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(State);
}

void SZHJZTagManagerWidget::NotifyUser(const FString& Msg, bool bSuccess) const
{
	Notify(Msg, bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
}

class SZHJZTagRow : public STableRow<SZHJZTagManagerWidget::FItem>
{
public:
	SLATE_BEGIN_ARGS(SZHJZTagRow)
		{
		}

		SLATE_ARGUMENT(TWeakPtr<SZHJZTagManagerWidget>, OwnerWidget)
		SLATE_ARGUMENT(SZHJZTagManagerWidget::FItem, Item)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
	{
		OwnerWidget = InArgs._OwnerWidget;
		Item = InArgs._Item;

		STableRow::Construct(
			STableRow::FArguments().Padding(FMargin(0.f, 0.f)),
			OwnerTable
		);

		ChildSlot
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(4, 2)
			[
				SAssignNew(InlineTag, SInlineEditableTextBlock)
				.Text_Lambda([WeakItem = TWeakPtr<FEventGameplayTagListItem>(Item)]()
				{
					const TSharedPtr<FEventGameplayTagListItem> Pinned = WeakItem.Pin();
					return FText::FromString(Pinned.IsValid() ? Pinned->Tag : TEXT(""));
				})
				.OnTextCommitted(this, &SZHJZTagRow::OnTagCommitted)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(4, 2)
			[
				SAssignNew(InlineComment, SInlineEditableTextBlock)
				.Text_Lambda([WeakItem = TWeakPtr<FEventGameplayTagListItem>(Item)]()
				{
					const TSharedPtr<FEventGameplayTagListItem> Pinned = WeakItem.Pin();
					return FText::FromString(Pinned.IsValid() ? Pinned->DevComment : TEXT(""));
				})
				.OnTextCommitted(this, &SZHJZTagRow::OnCommentCommitted)
			]
		];
	}

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		const FVector2D Local = InMyGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		const float HalfX = InMyGeometry.GetLocalSize().X * 0.5f;

		if (Local.X <= HalfX)
		{
			if (InlineTag.IsValid())
			{
				InlineTag->EnterEditingMode();
				return FReply::Handled();
			}
		}
		else
		{
			if (InlineComment.IsValid())
			{
				InlineComment->EnterEditingMode();
				return FReply::Handled();
			}
		}

		return STableRow::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
	}

private:
	void OnCommentCommitted(const FText& NewText, ETextCommit::Type CommitType)
	{
		const TSharedPtr<FEventGameplayTagListItem> Pinned = Item.Pin();
		if (!Pinned.IsValid())
		{
			return;
		}

		Pinned->DevComment = NewText.ToString();
	}

	void OnTagCommitted(const FText& NewText, ETextCommit::Type CommitType)
	{
		const TSharedPtr<FEventGameplayTagListItem> Pinned = Item.Pin();
		if (!Pinned.IsValid())
		{
			return;
		}

		const FString NewTagStr = NewText.ToString().TrimStartAndEnd();
		const FString OldTagStr = Pinned->Tag;

		if (NewTagStr == OldTagStr)
		{
			return;
		}

		const TSharedPtr<SZHJZTagManagerWidget> OwnerPinned = OwnerWidget.Pin();
		if (!OwnerPinned.IsValid())
		{
			return;
		}

		FString Error;
		if (!OwnerPinned->TryRenameItemTag(Item.Pin(), NewTagStr, Error))
		{
			if (!Error.IsEmpty())
			{
				Notify(Error, SNotificationItem::CS_Fail);
			}
			// 不修改情况下自动回显旧值
			// TO DO: 这里有点hack，后续考虑更优雅的方式
		}
	}

private:
	TWeakPtr<SZHJZTagManagerWidget> OwnerWidget;
	TWeakPtr<FEventGameplayTagListItem> Item;

	TSharedPtr<SInlineEditableTextBlock> InlineTag;
	TSharedPtr<SInlineEditableTextBlock> InlineComment;
};

void SZHJZTagManagerWidget::Construct(const FArguments& InArgs)
{
	ReloadFromDatatable();

	{
		FPropertyEditorModule& PropModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		FDetailsViewArgs DetailsArgs;
		DetailsArgs.bHideSelectionTip = true;
		DetailsArgs.bLockable = false;
		DetailsArgs.bUpdatesFromSelection = false;
		DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

		BarDetailsView = PropModule.CreateDetailView(DetailsArgs);
		BarDetailsView->SetObject(nullptr);
	}

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8)
		[
			SNew(SVerticalBox)

			// Toolbar row
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Reload")))
					.OnClicked(this, &SZHJZTagManagerWidget::OnReload)
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Apply")))
					.OnClicked(this, &SZHJZTagManagerWidget::OnApply)
				]
			]

			// Add row
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.45f).Padding(0, 0, 8, 0)
				[
					SAssignNew(NewTagTextBox, SEditableTextBox)
					.HintText(FText::FromString(TEXT("New Tag (e.g. EventTag.TopOptions.Init)")))
				]
				+ SHorizontalBox::Slot().FillWidth(0.45f).Padding(0, 0, 8, 0)
				[
					SAssignNew(NewCommentTextBox, SEditableTextBox)
					.HintText(FText::FromString(TEXT("DevComment (optional)")))
				]
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Add")))
					.OnClicked(this, &SZHJZTagManagerWidget::OnAdd)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Remove Selected")))
					.OnClicked(this, &SZHJZTagManagerWidget::OnRemoveSelected)
				]
			]

			// List
			+ SVerticalBox::Slot()
			.FillHeight(0.45f)
			.Padding(0, 0, 0, 8)
			[
				SAssignNew(ListView, SListView<FItem>)
				.ListItemsSource(&Items)
				.OnGenerateRow(this, &SZHJZTagManagerWidget::OnGenerateRow)
				.SelectionMode(ESelectionMode::Single)
			]

			// DataAsset editor
			+ SVerticalBox::Slot()
			.FillHeight(0.55f)
			[
				SNew(SBorder)
				.Padding(8)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 6)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0, 0, 8, 0)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Bar Config Asset")))
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(SObjectPropertyEntryBox)
							.AllowedClass(UBarConfigAssetBase::StaticClass())
							.ObjectPath_Lambda([this]()
							{
								const UObject* Obj = EditingBarAsset.Get();
								return Obj ? Obj->GetPathName() : FString();
							})
							.OnObjectChanged_Lambda([this](const FAssetData& AssetData)
							{
								UBarConfigAssetBase* Asset = Cast<UBarConfigAssetBase>(AssetData.GetAsset());
								SetEditingBarAsset(Asset);
							})
							.AllowClear(true)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(8, 0, 0, 0)
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("Save")))
							.OnClicked(this, &SZHJZTagManagerWidget::OnSaveBarAsset)
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 6)
					[
						SNew(SSeparator)
					]

					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						BarDetailsView.IsValid()
							? BarDetailsView.ToSharedRef()
							: SNullWidget::NullWidget
					]
				]
			]
		]
	];
}

void SZHJZTagManagerWidget::ReloadFromDatatable()
{
	TArray<FEventGameplayTagListItem> Loaded;
	FString Error;

	if (!FZHJZGameplayTagsIniUtils::LoadEventTagsFromDataTable(Loaded, &Error))
	{
		Notify(Error, SNotificationItem::CS_Fail);
		return;
	}

	Items.Reset();
	for (const FEventGameplayTagListItem& It : Loaded)
	{
		Items.Add(MakeShared<FEventGameplayTagListItem>(It));
	}

	if (ListView)
	{
		ListView->RequestListRefresh();
	}
}

FReply SZHJZTagManagerWidget::OnReload()
{
	ReloadFromDatatable();
	Notify(TEXT("Reloaded."), SNotificationItem::CS_Success);
	return FReply::Handled();
}

SZHJZTagManagerWidget::FItem SZHJZTagManagerWidget::GetSelectedItem() const
{
	if (!ListView)
	{
		return nullptr;
	}
	const TArray<FItem> Selected = ListView->GetSelectedItems();
	return Selected.Num() > 0 ? Selected[0] : nullptr;
}

void SZHJZTagManagerWidget::SetEditingBarAsset(UBarConfigAssetBase* NewAsset)
{
	EditingBarAsset = NewAsset;

	if (BarDetailsView.IsValid())
	{
		BarDetailsView->SetObject(NewAsset);
	}

	if (NewAsset)
	{
		Notify(FString::Printf(TEXT("Editing: %s"), *NewAsset->GetPathName()), SNotificationItem::CS_None);
	}
}

FReply SZHJZTagManagerWidget::OnSaveBarAsset()
{
	UBarConfigAssetBase* Asset = EditingBarAsset.Get();
	if (!Asset)
	{
		Notify(TEXT("No BarConfigAsset selected."), SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	Asset->MarkPackageDirty();

	if (UEditorAssetLibrary::SaveLoadedAsset(Asset, true))
	{
		Notify(TEXT("BarConfigAsset saved."), SNotificationItem::CS_Success);
	}
	else
	{
		Notify(TEXT("Failed to save BarConfigAsset."), SNotificationItem::CS_Fail);
	}

	return FReply::Handled();
}

FReply SZHJZTagManagerWidget::OnAdd()
{
	const FString NewTagStr = NewTagTextBox ? NewTagTextBox->GetText().ToString().TrimStartAndEnd() : TEXT("");
	const FString CommentStr = NewCommentTextBox ? NewCommentTextBox->GetText().ToString() : TEXT("");

	if (!FZHJZGameplayTagsIniUtils::IsValidGameplayTagString(NewTagStr))
	{
		Notify(TEXT("Invalid tag string."), SNotificationItem::CS_Fail);
		return FReply::Handled();
	}
	if (!FZHJZGameplayTagsIniUtils::IsEventTag(NewTagStr))
	{
		Notify(TEXT("Tag must be under EventTag."), SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	for (const FItem& Existing : Items)
	{
		if (Existing && Existing->Tag == NewTagStr)
		{
			Notify(TEXT("Tag already exists in list."), SNotificationItem::CS_Fail);
			return FReply::Handled();
		}
	}

	FItem NewItem = MakeShared<FEventGameplayTagListItem>();
	NewItem->Tag = NewTagStr;
	NewItem->DevComment = CommentStr;

	Items.Add(NewItem);
	Items.Sort([](const FItem& A, const FItem& B) { return A->Tag < B->Tag; });

	if (ListView)
	{
		ListView->RequestListRefresh();
		ListView->SetSelection(NewItem);
	}

	if (NewTagTextBox)
	{
		NewTagTextBox->SetText(FText::GetEmpty());
	}
	if (NewCommentTextBox)
	{
		NewCommentTextBox->SetText(FText::GetEmpty());
	}

	return FReply::Handled();
}

FReply SZHJZTagManagerWidget::OnRemoveSelected()
{
	const FItem Selected = GetSelectedItem();
	if (!Selected)
	{
		return FReply::Handled();
	}

	Items.Remove(Selected);

	if (ListView)
	{
		ListView->RequestListRefresh();
	}

	return FReply::Handled();
}

FReply SZHJZTagManagerWidget::OnApply()
{
	TArray<FEventGameplayTagListItem> Out;
	Out.Reserve(Items.Num());

	for (const FItem& It : Items)
	{
		if (!It)
		{
			continue;
		}
		Out.Add(*It);
	}

	FString Error;
	if (!FZHJZGameplayTagsIniUtils::SaveEventTagsToDataTable(Out, &Error))
	{
		Notify(Error, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	FZHJZGameplayTagsIniUtils::RefreshGameplayTagTreeInEditor();
	Notify(TEXT("Applied and refreshed tag tree."), SNotificationItem::CS_Success);

	return FReply::Handled();
}

bool SZHJZTagManagerWidget::TryRenameItemTag(const FItem& Item, const FString& NewTagStr, FString& OutError)
{
	OutError.Reset();

	if (!Item.IsValid())
	{
		OutError = TEXT("Invalid item.");
		return false;
	}

	const FString Trimmed = NewTagStr.TrimStartAndEnd();
	if (Trimmed.IsEmpty())
	{
		OutError = TEXT("Tag cannot be empty.");
		return false;
	}

	if (!FZHJZGameplayTagsIniUtils::IsValidGameplayTagString(Trimmed))
	{
		OutError = TEXT("Invalid tag string.");
		return false;
	}
	if (!FZHJZGameplayTagsIniUtils::IsEventTag(Trimmed))
	{
		OutError = TEXT("Tag must be under EventTag.");
		return false;
	}

	for (const FItem& Existing : Items)
	{
		if (!Existing.IsValid())
		{
			continue;
		}
		if (Existing == Item)
		{
			continue;
		}
		if (Existing->Tag == Trimmed)
		{
			OutError = TEXT("Tag already exists in list.");
			return false;
		}
	}

	Item->Tag = Trimmed;

	Items.Sort([](const FItem& A, const FItem& B)
	{
		if (!A.IsValid() || !B.IsValid())
		{
			return false;
		}
		return A->Tag < B->Tag;
	});

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
		ListView->SetSelection(Item);
	}

	return true;
}

TSharedRef<ITableRow> SZHJZTagManagerWidget::OnGenerateRow(FItem Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SZHJZTagRow, OwnerTable)
		.OwnerWidget(SharedThis(this))
		.Item(Item);
}
