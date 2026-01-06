#include "TagTools/ZHJZTagManagerWidget.h"

#include "TagTools/ZHJZGameplayTagsIniUtils.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/STextBlock.h"

static void Notify(const FString& Msg, SNotificationItem::ECompletionState State)
{
	FNotificationInfo Info(FText::FromString(Msg));
	Info.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(State);
}

void SZHJZTagManagerWidget::Construct(const FArguments& InArgs)
{
	ReloadFromIni();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8)
		[
			SNew(SVerticalBox)

			// Toolbar row
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0,0,0,8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
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
			.Padding(0,0,0,8)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(0.45f).Padding(0,0,8,0)
				[
					SAssignNew(NewTagTextBox, SEditableTextBox)
					.HintText(FText::FromString(TEXT("New Tag (e.g. ZHJZ.Event.Menu.Open)")))
				]
				+ SHorizontalBox::Slot().FillWidth(0.45f).Padding(0,0,8,0)
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
				+ SHorizontalBox::Slot().AutoWidth().Padding(8,0,0,0)
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Remove Selected")))
					.OnClicked(this, &SZHJZTagManagerWidget::OnRemoveSelected)
				]
			]

			// List
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(ListView, SListView<FItem>)
				.ListItemsSource(&Items)
				.OnGenerateRow(this, &SZHJZTagManagerWidget::OnGenerateRow)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	];
}

void SZHJZTagManagerWidget::ReloadFromIni()
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
	ReloadFromIni();
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
		Notify(TEXT("Tag must be under ZHJZ.Event."), SNotificationItem::CS_Fail);
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
	Items.Sort([](const FItem& A, const FItem& B){ return A->Tag < B->Tag; });

	if (ListView)
	{
		ListView->RequestListRefresh();
		ListView->SetSelection(NewItem);
	}

	if (NewTagTextBox) NewTagTextBox->SetText(FText::GetEmpty());
	if (NewCommentTextBox) NewCommentTextBox->SetText(FText::GetEmpty());

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
		if (!It) continue;
		Out.Add(*It);
	}

	FString Error;
	if (!FZHJZGameplayTagsIniUtils::SaveEventTagsToDataTable(Out, &Error))
	{
		Notify(Error, SNotificationItem::CS_Fail);
		return FReply::Handled();
	}

	FZHJZGameplayTagsIniUtils::RefreshGameplayTagTreeInEditor();
	Notify(TEXT("Applied to plugin DefaultGameplayTags.ini and refreshed tag tree."), SNotificationItem::CS_Success);

	return FReply::Handled();
}

TSharedRef<ITableRow> SZHJZTagManagerWidget::OnGenerateRow(FItem Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FItem>, OwnerTable)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		.Padding(4,2)
		[
			SNew(STextBlock)
			.Text_Lambda([Item]()
			{
				return FText::FromString(Item.IsValid() ? Item->Tag : TEXT(""));
			})
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		.Padding(4,2)
		[
			SNew(STextBlock)
			.Text_Lambda([Item]()
			{
				return FText::FromString(Item.IsValid() ? Item->DevComment : TEXT(""));
			})
		]
	];
}