// Copyright CengJia.  Year : 2022

#pragma once
#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/EngineTypes.h"
#include "Engine/DeveloperSettings.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION>=5 && ENGINE_MINOR_VERSION >= 2
	#include "RHITypes.h"
#endif
#include "opencv_handle.h"
#include "OpenCVPlayer.generated.h"
//----------------------------------------------------------------------------------------------------------------------
USTRUCT(BlueprintType)
struct FOpenCVPlayerSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	int AutoReconnectMaxCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	float ReconnectIntervalMinTime = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	float ReconnectIntervalMaxTime = 3.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="")
	int CustomFps = 0;
};

UENUM(BlueprintType)
enum EOpenCVlayerEvent
{
	OnOpenCVPlayerOpened,
	OnOpenCVPlayerStopped
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOpenCVPlayerEventDelegate,EOpenCVlayerEvent,Event);
//----------------------------------------------------------------------------------------------------------------------
UCLASS(ClassGroup = OpenCVPlayer, Blueprintable)
class  UOpenCVPlayer : public UObject
{
	GENERATED_BODY()
	
	FTimerHandle TimerHandle;

	FUpdateTextureRegion2D Region2D;
	
	std::shared_ptr<cv::VideoBuffer> VideoBufferSharedPtr = nullptr;

public:

	cv::VideoInfo VideoInfo;

	UPROPERTY(BlueprintAssignable,Category="OpenCVPlayer")
	FOpenCVPlayerEventDelegate OpenCVPlayerEventDelegate;
	
	UPROPERTY(BlueprintReadOnly,Category="OpenCVPlayer")
	UTexture2D* VideoTexture = nullptr;

	FOpenCVPlayerSettings OpenCVPlayerSettings;
	
public:
	
	UFUNCTION(BlueprintPure, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DisplayName = "Create OpenCVPlayer", CompactNodeTitle = "OpenCVPlayer", Keywords = " Create New OpenCVPlayer"), Category = "OpenCVPlayer")
	static UOpenCVPlayer* NewUOpenCVPlayer(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "OpenCVPlayer")
	void StartPlay(const FString& InVideoURL, FOpenCVPlayerSettings Settings);

	UFUNCTION(BlueprintCallable, Category = "OpenCVPlayer")
	void StopPlay();
	
	virtual void BeginDestroy() override;

	void UpdateTextureRegions();
	
	void Initialization(int Width, int Height, int Frame, std::shared_ptr<cv::VideoBuffer> VideoBufferSharedPtrParam);

	void BroadcastEvent(EOpenCVlayerEvent Event);

	void NeedReconnect(float IntervalMinTime);

	void Reconnect();
};