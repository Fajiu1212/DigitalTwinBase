#pragma once
#include "ExtendEditorStyle.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"


#define LOCTEXT_NAMESPACE "FExtendEditorStyleModule"


void FExtendEditorStyleModule::StartupModule()
{
	//InitializeStyle();
}

void FExtendEditorStyleModule::ShutdownModule()
{
	//FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
	//StyleSet.Reset();
}

FName FExtendEditorStyleModule::GetStyleSetName()
{
	UE_LOG(LogTemp, Warning, TEXT("GetStyleSetName"));
	return StyleSetName;
}

void FExtendEditorStyleModule::InitializeStyle()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
		UE_LOG(LogTemp, Warning, TEXT("RegisterSlateStyle"));
	}
}

TSharedRef<FSlateStyleSet> FExtendEditorStyleModule::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> SlateStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("QingyuanToolkits"))->GetBaseDir();
	const FString ResourcePath = FPaths::Combine(PluginDir,TEXT("Resources/"));

	SlateStyleSet->SetContentRoot(ResourcePath);

	{
		const FVector2D IconSize(16.f, 16.f);
		FSlateImageBrush* SlateImageBrush =
			new FSlateImageBrush(ResourcePath + TEXT("Fajiu.png"), IconSize);
		SlateStyleSet->Set("Fajiu.Slate", SlateImageBrush);
	}
	{
		const FVector2D IconeSize(16.f, 16.f);
		FSlateImageBrush* SlateImageBrush =
			new FSlateImageBrush(ResourcePath + TEXT("AliceIcon.png"), IconeSize);

		SlateStyleSet->Set("ExtendEditorCommands.PrintLog", SlateImageBrush);
	}
	return SlateStyleSet;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FExtendEditorStyleModule, ExtendEditorStyle)
