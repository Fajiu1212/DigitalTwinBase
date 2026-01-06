#pragma once

#include "CoreMinimal.h"

class EXTENDEDITORCOMMAND_API FExtendEditorCommands : public TCommands<FExtendEditorCommands>
{
public:
	FExtendEditorCommands();
	virtual void RegisterCommands() override;
	
	static const FExtendEditorCommands& Get();
	TSharedPtr<FUICommandInfo> PrintLog;
	static TSharedRef<FUICommandList> CommandList;
};

class EXTENDEDITORCOMMAND_API FExtendEditorActionCallbacks
{
public:
	static void PrintLog_Execute();
};