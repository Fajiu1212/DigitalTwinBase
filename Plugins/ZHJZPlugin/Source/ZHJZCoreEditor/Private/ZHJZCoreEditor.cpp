#include "ZHJZCoreEditor.h"

#include "GameplayTagsSettings.h"
#include "TagTools/ZHJZTagManagerWidget.h"

#include "Framework/Docking/TabManager.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

static const FName ZHJZTagManagerTabName(TEXT("ZHJZTagManager"));

#define LOCTEXT_NAMESPACE "FZHJZCoreEditorModule"

void FZHJZCoreEditorModule::StartupModule()
{
	RegisterZHJZEventTagsTable();
	
	// 1) 注册可打开的 Nomad Tab
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		ZHJZTagManagerTabName,
		FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
		{
			return SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				[
					SNew(SZHJZTagManagerWidget)
				];
		}))
		.SetDisplayName(LOCTEXT("ZHJZTagManagerTabTitle", "ZHJZ Tag Manager"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// 2) ToolMenus：在 menus 可用后注册工具栏按钮
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FZHJZCoreEditorModule::RegisterToolBarMenu)
	);
}

void FZHJZCoreEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ZHJZTagManagerTabName);
}

void FZHJZCoreEditorModule::RegisterToolBarMenu()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User"));
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection(TEXT("TagTools"));

	const FUIAction Action(FExecuteAction::CreateLambda([]()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(ZHJZTagManagerTabName);
	}));

	FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
		TEXT("ZHJZ.OpenTagManager"),
		Action,
		LOCTEXT("ZHJZOpenTagManager_Label", "ZHJZ Tags"),
		LOCTEXT("ZHJZOpenTagManager_Tooltip", "Open ZHJZ Tag Manager"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Tool")
	);

	Section.AddEntry(Entry);
}

void FZHJZCoreEditorModule::RegisterZHJZEventTagsTable()
{
	// 你的 DataTable 软引用
	const FSoftObjectPath TablePath(TEXT("/ZHJZPlugin/DataTable/DT_ZHJZEventTags.DT_ZHJZEventTags"));

	UGameplayTagsSettings* Settings = GetMutableDefault<UGameplayTagsSettings>();
	if (!Settings)
	{
		return;
	}

	// GameplayTagTableList 类型是 TArray<FSoftObjectPath>（或 TArray<TSoftObjectPtr<UDataTable>>，不同版本略有差异）
	// 这里用最兼容的方式：把 SoftObjectPath 加进去
	bool bAlreadyAdded = false;
	for (const FSoftObjectPath& Existing : Settings->GameplayTagTableList)
	{
		if (Existing == TablePath)
		{
			bAlreadyAdded = true;
			break;
		}
	}

	if (!bAlreadyAdded)
	{
		Settings->GameplayTagTableList.Add(TablePath);
		Settings->SaveConfig();
	}

	// 刷新 tag tree
	UGameplayTagsManager::Get().EditorRefreshGameplayTagTree();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZHJZCoreEditorModule, ZHJZCoreEditor)