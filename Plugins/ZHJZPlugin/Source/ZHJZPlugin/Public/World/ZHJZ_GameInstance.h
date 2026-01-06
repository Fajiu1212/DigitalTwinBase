#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "ZHJZ_GameInstance.generated.h"

class ULevelStreamingDynamic;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelChangedDelegate, UWorld*, LoadedWorld);

UCLASS()
class ZHJZPLUGIN_API UZHJZ_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UZHJZ_GameInstance(const class FObjectInitializer&);

	virtual void Init() override;
	virtual void Shutdown() override;

	//Getter
	UFUNCTION(BlueprintPure, Category = "AwsaGameInstance")
	static UZHJZ_GameInstance* GetAwsaGameInstance();

	//Streaming level
	UFUNCTION(BlueprintCallable, Category="AwsaGameInstance|WorldStreaming")
	bool LoadLevelByStreamLevel(const TSoftObjectPtr<UWorld> TargetLevel);
	UFUNCTION(BlueprintCallable, Category="AwsaGameInstance|WorldStreaming")
	bool UnLoadLevelByStreamLevel(const TSoftObjectPtr<UWorld> TargetLevel);
	UFUNCTION(BlueprintCallable, Category="AwsaGameInstance|WorldStreaming")
	bool LoadLevelByLevelInstance(const TSoftObjectPtr<UWorld> TargetLevel,
	                              ULevelStreamingDynamic*& LevelStreamingDynamic);
	UPROPERTY(BlueprintAssignable, Category="AwsaGameInstance|WorldStreaming")
	FOnLevelChangedDelegate OnLevelChanged;
	UFUNCTION()
	void OnLevelLoaded();

private:
	//Pc time
	int32 TimeValue = 750;
	void RefreshTime();
	UPROPERTY()
	FTimerHandle PcTimeHandle;
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "AwsaGameInstance|PcTime")
	int32 GetNow();
};
