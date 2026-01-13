#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FZHJZCoreEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	inline static TSharedPtr<FSlateStyleSet> StyleSet = nullptr;
	inline static const FName StyleSetName = FName("AWSAEditorStyle");

public:
	static ZHJZCOREEDITOR_API FName GetStyleSetName();

private:
	void InitializeStyle();
	TSharedRef<FSlateStyleSet> CreateSlateStyleSet();

public:
	//Tool bar menu
	void RegisterToolBarMenu();
	void RegisterZHJZEventTagsTable();

	void FillToolBarMenu(UToolMenu* Menu);
};
