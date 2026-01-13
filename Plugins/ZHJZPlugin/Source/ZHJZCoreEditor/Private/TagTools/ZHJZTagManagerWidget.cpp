#include "TagTools/ZHJZTagManagerWidget.h"

#include "TagTools/ZHJZGameplayTagsIniUtils.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/STextBlock.h"

// DataAsset editor
#include "MainUI/TopBarConfigAsset.h"
#include "EditorAssetLibrary.h"
#include "IContentBrowserSingleton.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

#include "PropertyCustomizationHelpers.h"
#include "Widgets/Layout/SSeparator.h"

static void Notify(const FString& Msg, SNotificationItem::ECompletionState State)
{
	FNotificationInfo Info(FText::FromString(Msg));
	Info.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(State);
}

void SZHJZTagManagerWidget::Construct(const FArguments& InArgs)
{
	ReloadFromDatatable();

	// Create DetailsView
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

	// Create AssetPicker
	FAssetPickerConfig Picker;
	Picker.SelectionMode = ESelectionMode::Single;
	Picker.bAllowNullSelection = true;
	Picker.InitialAssetViewType = EAssetViewType::List;
	Picker.Filter.ClassPaths.Add(UBarConfigAssetBase::StaticClass()->GetClassPathName());
	Picker.OnAssetSelected = FOnAssetSelected::CreateLambda([this](const FAssetData& AssetData)
	{
		UBarConfigAssetBase* Asset = Cast<UBarConfigAssetBase>(AssetData.GetAsset());
		SetEditingBarAsset(Asset);
	});

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

			// List (top part)
			+ SVerticalBox::Slot()
			.FillHeight(0.45f)
			.Padding(0, 0, 0, 8)
			[
				SAssignNew(ListView, SListView<FItem>)
				.ListItemsSource(&Items)
				.OnGenerateRow(this, &SZHJZTagManagerWidget::OnGenerateRow)
				.SelectionMode(ESelectionMode::Single)
			]

			// DataAsset editor (bottom part)
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
								                             UBarConfigAssetBase* Asset = Cast<UBarConfigAssetBase>(
									                             AssetData.GetAsset());
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
		//UTopBarWidgetBase::RefreshAllByConfig(Asset);
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

TSharedRef<ITableRow> SZHJZTagManagerWidget::OnGenerateRow(FItem Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FItem>, OwnerTable)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(4, 2)
			[
				SNew(STextBlock)
				.Text_Lambda([Item]()
				{
					return FText::FromString(Item.IsValid() ? Item->Tag : TEXT(""));
				})
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(4, 2)
			[
				SNew(STextBlock)
				.Text_Lambda([Item]()
				{
					return FText::FromString(Item.IsValid() ? Item->DevComment : TEXT(""));
				})
			]
		];
}
