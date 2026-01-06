#include "ExtendEditorCommand.h"

#include "ExtendEditorAction.h"

#define LOCTEXT_NAMESPACE "FExtendEditorCommandModule"

void FExtendEditorCommandModule::StartupModule()
{
	FExtendEditorCommands::Register();
}

void FExtendEditorCommandModule::ShutdownModule()
{
	FExtendEditorCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FExtendEditorCommandModule, ExtendEditorCommand)
