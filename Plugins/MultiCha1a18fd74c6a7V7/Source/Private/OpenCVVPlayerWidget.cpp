// Copyright CengJia.  Year : 2022

#include "OpenCVPlayerWidget.h"

void UOpenCVPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitOpenCVPlayer();
	
	if(bAutoPlay && InitialVideoURL.Len() > 0)
		StartPlay(InitialVideoURL,LoadingImageTexture,true,OpenCVPlayerSettings);
}

void UOpenCVPlayerWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	StopPlay();
}

void UOpenCVPlayerWidget::BeginDestroy()
{
	Super::BeginDestroy();
	
	StopPlay();
	
	OpenCVPlayer = nullptr;
}

void UOpenCVPlayerWidget::InitOpenCVPlayer()
{
	if(OpenCVPlayer == nullptr)
		OpenCVPlayer = UOpenCVPlayer::NewUOpenCVPlayer(this);
}

void UOpenCVPlayerWidget::StopPlay()
{
	if(OpenCVPlayer)
		OpenCVPlayer->StopPlay();
}

void UOpenCVPlayerWidget::StartPlay(const FString& InVideoURL, UTexture2D* LoadingImage, bool bShowLoading,FOpenCVPlayerSettings Settings)
{
	if(IsValidLowLevelFast())
	{
		if(bFirstOpen || bShowLoading)
		{
			if(LoadingImage)
				ImageVideo->SetBrushFromTexture( LoadingImage,true );
			else if(LoadingImageTexture)
				ImageVideo->SetBrushFromTexture( LoadingImageTexture,true );
		}
		bFirstOpen = false;
	
		if(OpenCVPlayer)
			OpenCVPlayer->StartPlay(InVideoURL,Settings);
		else
			UE_LOG(LogTemp,Warning,TEXT("OpenCVPlayer is nullptr"));
	}
}

UTexture2D* UOpenCVPlayerWidget::GetTexture()
{
	if(OpenCVPlayer)
		return OpenCVPlayer->VideoTexture;

	return nullptr;
}