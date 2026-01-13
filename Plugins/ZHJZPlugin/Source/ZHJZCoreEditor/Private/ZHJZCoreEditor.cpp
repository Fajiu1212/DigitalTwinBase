#include "ZHJZCoreEditor.h"

#include "GameplayTagsSettings.h"
#include "TagTools/ZHJZTagManagerWidget.h"

#include "Framework/Docking/TabManager.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Docking/SDockTab.h"

static const FName EventTagManager(TEXT("EventTagManager"));

#define LOCTEXT_NAMESPACE "FZHJZCoreEditorModule"

void FZHJZCoreEditorModule::StartupModule()
{
	InitializeStyle();
	RegisterZHJZEventTagsTable();

	// 注册 Nomad Tab
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		                        EventTagManager,
		                        FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
		                        {
		                        	TSharedRef<SDockTab> Tab = SNew(SDockTab)
				                        .TabRole(ETabRole::NomadTab)
				                        [
					                        SNew(SZHJZTagManagerWidget)
				                        ];
		                        	Tab->SetTabIcon(FSlateIcon(GetStyleSetName(), "Fajiu.Slate").GetIcon());
		                        	return Tab;
		                        }))
	                        .SetDisplayName(LOCTEXT("AWSATagManagerTabTitle", "AWSA Tag Manager"))
	                        .SetMenuType(ETabSpawnerMenuType::Hidden);

	// 注册 toolmenu button
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FZHJZCoreEditorModule::RegisterToolBarMenu)
	);
}

void FZHJZCoreEditorModule::ShutdownModule()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
	StyleSet.Reset();
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(EventTagManager);
}

FName FZHJZCoreEditorModule::GetStyleSetName()
{
	return StyleSetName;
}

void FZHJZCoreEditorModule::InitializeStyle()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
	}
}

TSharedRef<FSlateStyleSet> FZHJZCoreEditorModule::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> SlateStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("ZHJZPlugin"))->GetBaseDir();
	const FString ResourcePath = FPaths::Combine(PluginDir,TEXT("Resources/"));

	SlateStyleSet->SetContentRoot(ResourcePath);

	{
		const FVector2D IconSize(16.f, 16.f);
		FSlateImageBrush* SlateImageBrush =
			new FSlateImageBrush(ResourcePath + TEXT("Fajiu.png"), IconSize);
		SlateStyleSet->Set("Fajiu.Slate", SlateImageBrush);
	}
	return SlateStyleSet;
}

void FZHJZCoreEditorModule::RegisterToolBarMenu()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User"));
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection(TEXT("TagTools"));

	Section.AddEntry(FToolMenuEntry::InitComboButton("AWSA Editor Toolkit", FToolUIAction(),
	                                                 FNewToolMenuDelegate::CreateRaw(
		                                                 this, &FZHJZCoreEditorModule::FillToolBarMenu),
	                                                 LOCTEXT("AWSA Editor Toolkit", "AWSA Editor Toolkit"),
	                                                 LOCTEXT("AWSA Editor Toolkit", "Toolkit menu"),
	                                                 FSlateIcon(GetStyleSetName(),
	                                                            "Fajiu.Slate"))
	);
}

void FZHJZCoreEditorModule::RegisterZHJZEventTagsTable()
{
	// tag DT soft ref.
	const FSoftObjectPath TablePath(TEXT("/ZHJZPlugin/DataTable/DT_ZHJZEventTags.DT_ZHJZEventTags"));

	UGameplayTagsSettings* Settings = GetMutableDefault<UGameplayTagsSettings>();
	if (!Settings)
	{
		return;
	}

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

void FZHJZCoreEditorModule::FillToolBarMenu(UToolMenu* Menu)
{
	FToolMenuSection& SubSection = Menu->AddSection("MainSection");

	const FUIAction Action(FExecuteAction::CreateLambda([]()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(EventTagManager);
	}));

	SubSection.AddEntry(
		FToolMenuEntry::InitMenuEntry(
			"Event Tag Manager",
			LOCTEXT("Event Tag Manager Label", "Event Tag Manager"),
			LOCTEXT("Event Tag Manager Tooltip", "Open event tag manager"),
			FSlateIcon(),
			Action
		)
	);
	SubSection.AddEntry(
		FToolMenuEntry::InitMenuEntry(
			"SubButton2",
			LOCTEXT("SubButton2Label", "Special Action"),
			LOCTEXT("SubButton2Tooltip", "Custom Functionality"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([]()
				{
					UE_LOG(LogTemp, Warning, TEXT("Custom Action Triggered!"));
				})
			)
		)
	);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZHJZCoreEditorModule, ZHJZCoreEditor)
