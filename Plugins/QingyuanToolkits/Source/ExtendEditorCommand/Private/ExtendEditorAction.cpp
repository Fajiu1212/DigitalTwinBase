#include "ExtendEditorAction.h"

#include "ExtendEditorStyle.h"
#include "Framework/Commands/UICommandList.h"

#define LOCTEXT_NAMESPACE "ExtendEditorCommands"

TSharedRef<FUICommandList> FExtendEditorCommands::CommandList = MakeShareable(new FUICommandList);

FExtendEditorCommands::FExtendEditorCommands()
	: TCommands<FExtendEditorCommands>(
		TEXT("ExtendEditorCommands"),
		LOCTEXT("ExtendEditor", "Extend Editor"),
		NAME_None,
		FExtendEditorStyleModule::GetStyleSetName()
	)
{
}

const FExtendEditorCommands& FExtendEditorCommands::Get()
{
	return TCommands<FExtendEditorCommands>::Get();
}

void FExtendEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		PrintLog,
		"Print Log",
		"Output debug message to log",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::L)
	);

	CommandList->MapAction(
		PrintLog,
		FExecuteAction::CreateStatic(&FExtendEditorActionCallbacks::PrintLog_Execute)
	);
}

#undef LOCTEXT_NAMESPACE

void FExtendEditorActionCallbacks::PrintLog_Execute()
{
	UE_LOG(LogTemp, Warning, TEXT("[ExtendEditor] Command executed!"));
}
