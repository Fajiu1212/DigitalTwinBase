#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


class FExtendEditorStyleModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual ~FExtendEditorStyleModule() override = default;

private:
	inline static TSharedPtr<FSlateStyleSet> StyleSet = nullptr;
	inline static const FName StyleSetName = FName("ExtendEditorStyle");

public:
	static EXTENDEDITORSTYLE_API FName GetStyleSetName();

private:
	void InitializeStyle();
	TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
};

inline static TSharedPtr<FExtendEditorStyleModule> ExtendEditorStyleModule = nullptr;
