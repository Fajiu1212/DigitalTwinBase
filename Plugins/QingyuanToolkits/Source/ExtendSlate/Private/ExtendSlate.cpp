#include "ExtendSlate.h"

#include "ToolMenus.h"
#include "ExtendEditorAction.h"
#include "ExtendEditorStyle.h"

#define LOCTEXT_NAMESPACE "FExtendSlateModule"

void FExtendSlateModule::StartupModule()
{
	IModuleInterface::StartupModule();
	//CreateToolBarMenu();
}

void FExtendSlateModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

void FExtendSlateModule::CreateToolBarMenu()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& Section = Menu->FindOrAddSection(NAME_None);

	FToolMenuEntry& MainEntry = Section.AddEntry(
		FToolMenuEntry::InitComboButton(
			"QingyuanToolBarButton",
			FUIAction(),
			FNewToolMenuDelegate::CreateRaw(this, &FExtendSlateModule::FillToolBarMenu),
			LOCTEXT("MainButtonLabel", "Qingyuan ToolKits"),
			LOCTEXT("MainButtonTooltip", "Qingyuan Editor Toolkits"),
			FSlateIcon(FExtendEditorStyleModule::GetStyleSetName(), "Fajiu.Slate")
		)
	);
}

void FExtendSlateModule::FillToolBarMenu(UToolMenu* Menu)
{
	FToolMenuSection& SubSection = Menu->AddSection("MainSection");

	SubSection.AddEntry(
		FToolMenuEntry::InitMenuEntry(
			"SaveCameraData",
			LOCTEXT("SubButton1Label", "Save Camera Data"),
			LOCTEXT("SubButton1Tooltip", "Save Current Camera Transform & Spring Arm Distance"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&FExtendEditorActionCallbacks::PrintLog_Execute)
			)
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

void FExtendSlateModule::SpawnTab_CameraData()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FExtendSlateModule, ExtendSlate)
