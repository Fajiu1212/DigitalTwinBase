// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/SettingsManager.h"

#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameUserSettings.h"

void USettingsManager::InitSettingManager()
{
	if (GEngine && GEngine->GetGameUserSettings())
	{
		UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
		initPoint = UserSettings->GetScreenResolution();
		initWindowMode = UserSettings->GetFullscreenMode();
		initGraphicsPreset = EAwsaGraphicsPreset::Low;
		SetGraphicsPreset(initGraphicsPreset);
	}
}

void USettingsManager::ChangeResolution(const FIntPoint& Point, EWindowMode::Type WindowMode)
{
	if (Point.X <= 0 || Point.Y <= 0)
	{
		return;
	}
	if (!GEngine)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	if (UserSettings->GetScreenResolution() == Point &&
		UserSettings->GetFullscreenMode() == WindowMode)
	{
		return;
	}

	UserSettings->SetScreenResolution(Point);
	UserSettings->SetFullscreenMode(WindowMode);


	UserSettings->ApplySettings(false);

	FSystemResolution::RequestResolutionChange(Point.X, Point.Y, WindowMode);
}


TArray<FScreenResolutionInfo> USettingsManager::FilterResolutionsByAspectRatio(float TargetAspectRatio, float Tolerance)
{
	const TArray<FScreenResolutionInfo> Resolutions = GetAllScreenResolutions();
	TArray<FScreenResolutionInfo> Filtered;
	for (const auto& Res : Resolutions)
	{
		if (FMath::Abs(Res.AspectRatio - TargetAspectRatio) <= Tolerance)
		{
			Filtered.Add(Res);
		}
	}
	return Filtered;
}

void USettingsManager::SetGraphicsPreset(EAwsaGraphicsPreset Preset)
{
	static const TMap<EAwsaGraphicsPreset, FString> ConsoleCommands = {
		{
			EAwsaGraphicsPreset::Low,
			"SCALABILITY 0; r.ScreenPercentage 100; r.PostProcessAAQuality 0; sg.PostProcessQuality 0; sg.ShadowQuality 0; sg.TextureQuality 0; sg.EffectsQuality 0; r.DetailMode 0"
		},
		{
			EAwsaGraphicsPreset::Medium,
			"SCALABILITY 1; r.ScreenPercentage 100; r.PostProcessAAQuality 1; sg.PostProcessQuality 1; sg.ShadowQuality 1; sg.TextureQuality 1; sg.EffectsQuality 1; r.DetailMode 1"
		},
		{
			EAwsaGraphicsPreset::High,
			"SCALABILITY 3; r.ScreenPercentage 100; r.PostProcessAAQuality 3; sg.PostProcessQuality 3; sg.ShadowQuality 3; sg.TextureQuality 3; sg.EffectsQuality 3; r.DetailMode 2"
		},
		{
			EAwsaGraphicsPreset::Epic,
			"SCALABILITY 3; r.ScreenPercentage 100; r.PostProcessAAQuality 3; sg.PostProcessQuality 3; sg.ShadowQuality 3; sg.TextureQuality 3; sg.EffectsQuality 3; r.DetailMode 2"
		}
	};

	if (const FString* Commands = ConsoleCommands.Find(Preset))
	{
		TArray<FString> CommandArray;
		Commands->ParseIntoArray(CommandArray, TEXT(";"), true);

		for (const FString& Command : CommandArray)
		{
			const FString TrimmedCommand = Command.TrimStartAndEnd();
			if (!TrimmedCommand.IsEmpty())
			{
				GEngine->Exec(nullptr, *TrimmedCommand);
			}
		}
	}
}

void USettingsManager::ReSetResolution()
{
	if (!GEngine)
	{
		return;
	}

	UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	if (UserSettings->GetScreenResolution() == initPoint &&
		UserSettings->GetFullscreenMode() == initWindowMode)
	{
		return;
	}

	UserSettings->SetScreenResolution(initPoint);
	UserSettings->SetFullscreenMode(initWindowMode);


	UserSettings->ApplySettings(false);

	FSystemResolution::RequestResolutionChange(initPoint.X, initPoint.Y, initWindowMode);
}

TArray<FScreenResolutionInfo> USettingsManager::GetAllScreenResolutions()
{
	TArray<FScreenResolutionInfo> Results;

	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	for (FIntPoint res : Resolutions)
	{
		FScreenResolutionInfo NewRes;
		NewRes.Width = res.X;
		NewRes.Height = res.Y;
		NewRes.AspectRatio = static_cast<float>(NewRes.Width) / NewRes.Height;
		Results.AddUnique(NewRes);
	}
	return Results;
}
