#pragma once

#include "CoreMinimal.h"
#include "Manager/BaseManager.h"
#include "SettingsManager.generated.h"


USTRUCT(BlueprintType)
struct FScreenResolutionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Width;

	UPROPERTY(BlueprintReadOnly)
	int32 Height;

	UPROPERTY(BlueprintReadOnly)
	float AspectRatio;

	bool operator==(const FScreenResolutionInfo& Other) const
	{
		return Width == Other.Width &&
			Height == Other.Height &&
			FMath::IsNearlyEqual(AspectRatio, Other.AspectRatio);
	}
};

UENUM(BlueprintType)
enum class EAwsaGraphicsPreset : uint8 {
	Low      UMETA(DisplayName = "Low Quality"),
	Medium   UMETA(DisplayName = "Medium Quality"),
	High     UMETA(DisplayName = "High Quality"),
	Epic     UMETA(DisplayName = "Epic Quality")
};

/**
 * 
 */
UCLASS()
class ZHJZPLUGIN_API USettingsManager : public UBaseManager
{
	GENERATED_BODY()

	public:

	    UFUNCTION(BlueprintCallable, Category = "System Setting")
		void InitSettingManager();

	    //change resolution
		UFUNCTION(BlueprintCallable, Category = "System Setting")
		void ChangeResolution(const FIntPoint& Point,  EWindowMode::Type WindowMode);

		UFUNCTION(BlueprintCallable, Category = "System Setting")
		void  ReSetResolution();
   
	    UFUNCTION(BlueprintPure, Category = "System Setting")
		static TArray<FScreenResolutionInfo> FilterResolutionsByAspectRatio(float TargetAspectRatio,float Tolerance = 0.01f);

		UFUNCTION(BlueprintCallable, Category = "System Setting")
		static void SetGraphicsPreset(EAwsaGraphicsPreset Preset);

	private:

		 static TArray<FScreenResolutionInfo> GetAllScreenResolutions();

		 EWindowMode::Type initWindowMode;

		 FIntPoint initPoint;
	 
		 EAwsaGraphicsPreset initGraphicsPreset;
};
