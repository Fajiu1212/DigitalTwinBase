// Copyright CengJia.  Year : 2022

#pragma once

#include "CoreMinimal.h"
#include "OpenCVPlayer.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"

#include "OpenCVPlayerWidget.generated.h"

UCLASS()
class UOpenCVPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

	bool bFirstOpen = true;
public:
	
	UPROPERTY(EditAnywhere, Category="OpenCVPlayerWidget")
	FString InitialVideoURL = "";

	UPROPERTY(EditAnywhere, Meta = (BindWidget), Category = "OpenCVPlayerWidget")
	bool bAutoPlay = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCVPlayerWidget")
	FOpenCVPlayerSettings OpenCVPlayerSettings;
	
	UPROPERTY(BlueprintReadOnly, Category="OpenCVPlayerWidget")
	UOpenCVPlayer* OpenCVPlayer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCVPlayerWidget")
	UTexture2D* LoadingImageTexture = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget), Category = "OpenCVPlayerWidget")
	UImage* ImageVideo;
	
public:

	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category="OpenCVPlayerWidget")
	void InitOpenCVPlayer();
	
	UFUNCTION(BlueprintCallable, Category="OpenCVPlayerWidget")
	void StartPlay(const FString& InVideoURL, UTexture2D* LoadingImage, bool bShowLoading,FOpenCVPlayerSettings Settings);

    UFUNCTION(BlueprintCallable, Category="OpenCVPlayerWidget")
	void StopPlay();
	
	UFUNCTION(BlueprintCallable, Category="OpenCVPlayerWidget")
	UTexture2D* GetTexture();
};