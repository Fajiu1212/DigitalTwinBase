// Copyright CengJia.  Year : 2022

#include "OpenCVPlayer.h"
#include "TimerManager.h"
#include "Engine.h"
#include "Async/Async.h"
#include "Interfaces/IPluginManager.h"

UOpenCVPlayer* UOpenCVPlayer::NewUOpenCVPlayer(UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	UOpenCVPlayer* OpenCVPlayer = NewObject<UOpenCVPlayer>(WorldContextObject);

	if(OpenCVPlayer)
	{
		OpenCVPlayer->VideoInfo.VideoOpenedDelegate = [OpenCVPlayer](int Width, int Height, int Frame, std::shared_ptr<cv::VideoBuffer> VideoBufferSP)
		{
			AsyncTask(ENamedThreads::GameThread, [OpenCVPlayer,Width,Height,Frame,VideoBufferSP]()
			{
				if(OpenCVPlayer->IsValidLowLevel() && OpenCVPlayer->VideoInfo.Ptr)
					OpenCVPlayer->Initialization(Width,Height,Frame,VideoBufferSP);
			});
		};

		OpenCVPlayer->VideoInfo.OpenCVEventDelegate = [OpenCVPlayer](cv::EOpenCVEvent Event)
		{
			switch (Event)
			{
			case cv::EOpenCVEvent::OnVideoStoped:
				OpenCVPlayer->BroadcastEvent(OnOpenCVPlayerStopped);
				break;
			case cv::EOpenCVEvent::OnNeedReconnect:
				if(OpenCVPlayer->IsValidLowLevel())
					OpenCVPlayer->NeedReconnect(FMath::RandRange(OpenCVPlayer->OpenCVPlayerSettings.ReconnectIntervalMinTime, OpenCVPlayer->OpenCVPlayerSettings.ReconnectIntervalMaxTime) );
				break;
			case cv::EOpenCVEvent::OnUpdate:
				OpenCVPlayer->UpdateTextureRegions();
				break;
			}
		};
		
		return OpenCVPlayer;
	}

	return nullptr;
}

void UOpenCVPlayer::NeedReconnect(float IntervalMinTime)
{
	if(IsValidLowLevel())
	{
		AsyncTask(ENamedThreads::GameThread, [this,IntervalMinTime]()
		{
			if(IsValidLowLevel())
			{
				GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UOpenCVPlayer::Reconnect, IntervalMinTime, false);
			}
		});
	}
}

void UOpenCVPlayer::Reconnect()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	cv::Reconnect(VideoInfo.Ptr);
}

void UOpenCVPlayer::StartPlay(const FString& InVideoURL, FOpenCVPlayerSettings Settings)
{
	if (InVideoURL.IsEmpty())
		return;
	
	StopPlay();
	
	OpenCVPlayerSettings = Settings;
	
	VideoInfo.VideoURL				= TCHAR_TO_UTF8(*InVideoURL) ;
	VideoInfo.Ptr					= nullptr;
	VideoInfo.CustomFps				= Settings.CustomFps;
	VideoInfo.bUseCustomFps			= Settings.CustomFps > 0;
	VideoInfo.AutoReconnectMaxCount	= Settings.AutoReconnectMaxCount;
	
	cv::PlayVideo(VideoInfo);
}

void UOpenCVPlayer::StopPlay()
{
	if(VideoInfo.Ptr)
	{
		cv::StopVideo(VideoInfo.Ptr);
		VideoInfo.Ptr = nullptr;
	}

	if(VideoBufferSharedPtr)
	{
		VideoBufferSharedPtr.reset();
		VideoBufferSharedPtr = nullptr;
	}
	
	VideoTexture = nullptr;
	
	if(GetWorld())
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void UOpenCVPlayer::BeginDestroy()
{
	UObject::BeginDestroy();
	
	StopPlay();

	OpenCVPlayerEventDelegate.Clear();
}

void UOpenCVPlayer::UpdateTextureRegions()
{
	// AsyncTask(ENamedThreads::GameThread, [this]()
	// {
	// 	if(VideoBufferSharedPtr && IsValidLowLevelFast() && VideoTexture && VideoTexture->IsValidLowLevelFast())
	// 	{
	// 		std::shared_ptr<cv::VideoBuffer> VideoBuffer = VideoBufferSharedPtr;
	// 		if(VideoBufferSharedPtr)
	// 			VideoTexture->UpdateTextureRegions(0, 1,	&Region2D, Region2D.Width * 4, 4, VideoBuffer->Buffer,[VideoBuffer](uint8* SrcData, const FUpdateTextureRegion2D* Regions){});
	// 	}
	// });
	AsyncTask(ENamedThreads::GameThread, [this]()
   {
	   if (VideoBufferSharedPtr && IsValidLowLevelFast() && VideoTexture && VideoTexture->IsValidLowLevelFast())
	   {
		   std::shared_ptr<cv::VideoBuffer> VideoBuffer = VideoBufferSharedPtr;
		   if (VideoBuffer && VideoBuffer->Buffer)
		   {
			   VideoTexture->UpdateTextureRegions(0, 1, &Region2D, Region2D.Width * 4, 4, VideoBuffer->Buffer, [VideoBuffer](uint8* SrcData, const FUpdateTextureRegion2D* Regions) {});
		   }
	   }
   });
}

void UOpenCVPlayer::Initialization(int Width, int Height, int Frame, std::shared_ptr<cv::VideoBuffer> VideoBufferSharedPtrParam)
{
	Region2D.SrcX = 0;
	Region2D.SrcY = 0;
	Region2D.DestX = 0;
	Region2D.DestY = 0;
	Region2D.Width = Width;
	Region2D.Height = Height;
			
	VideoBufferSharedPtr = VideoBufferSharedPtrParam;
	VideoTexture = UTexture2D::CreateTransient(Width, Height);
	VideoTexture->UpdateResource();
	
	BroadcastEvent(OnOpenCVPlayerOpened);
}

void UOpenCVPlayer::BroadcastEvent(EOpenCVlayerEvent Event)
{
	if (IsValidLowLevel())
	{
		if(IsInGameThread())
		{
			if(OpenCVPlayerEventDelegate.IsBound())
				OpenCVPlayerEventDelegate.Broadcast(Event);

			if(Event == EOpenCVlayerEvent::OnOpenCVPlayerStopped)
				VideoInfo.Ptr = nullptr;
		}
		else
		{
			AsyncTask(ENamedThreads::GameThread, [this,Event]()
			{
				if(IsValidLowLevel())
				{
					if(OpenCVPlayerEventDelegate.IsBound())
						OpenCVPlayerEventDelegate.Broadcast(Event);
			
					if(Event == EOpenCVlayerEvent::OnOpenCVPlayerStopped)
						VideoInfo.Ptr = nullptr;
				}
			});
		}
	}
}