#include "World/ZHJZ_GameInstance.h"

#include "AutoParse.h"
#include "TimerManager.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Kismet/GameplayStatics.h"


UZHJZ_GameInstance::UZHJZ_GameInstance(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}


void UZHJZ_GameInstance::Init()
{
	Super::Init();
	RefreshTime();
	GetWorld()->GetTimerManager().SetTimer(PcTimeHandle,this, &UZHJZ_GameInstance::RefreshTime, 60.f, true, -1);
}

void UZHJZ_GameInstance::Shutdown()
{
	Super::Shutdown();
}

UZHJZ_GameInstance* UZHJZ_GameInstance::GetAwsaGameInstance()
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		return Cast<UZHJZ_GameInstance>(World->GetGameInstance());
	}
	return nullptr;
}

bool UZHJZ_GameInstance::LoadLevelByStreamLevel(const TSoftObjectPtr<UWorld> TargetLevel)
{
	if (TargetLevel.IsNull())
	{
		UAutoParse::PrintLog_GameThread(TEXT("Invalid soft reference to UWorld"));
		return false;
	}
	FLatentActionInfo LatentInfo;
	LatentInfo.ExecutionFunction = "OnLevelLoaded";
	LatentInfo.UUID = FMath::Rand();
	LatentInfo.Linkage = 0;
	LatentInfo.CallbackTarget = this;
	UGameplayStatics::LoadStreamLevelBySoftObjectPtr(this, TargetLevel, true, true, LatentInfo);
	GetWorld()->FlushLevelStreaming();
	return true;
}

bool UZHJZ_GameInstance::UnLoadLevelByStreamLevel(const TSoftObjectPtr<UWorld> TargetLevel)
{
	if (TargetLevel.IsNull())
	{
		UAutoParse::PrintLog_GameThread(TEXT("Invalid soft reference to UWorld"));
		return false;
	}
	FLatentActionInfo LatentInfo;
	UGameplayStatics::UnloadStreamLevelBySoftObjectPtr(this, TargetLevel, LatentInfo, true);
	GetWorld()->FlushLevelStreaming();
	return true;
}

bool UZHJZ_GameInstance::LoadLevelByLevelInstance(const TSoftObjectPtr<UWorld> TargetLevel,
                                                  ULevelStreamingDynamic*& LevelStreamingDynamic)
{
	if (TargetLevel.IsNull())
	{
		UAutoParse::PrintLog_GameThread(TEXT("Invalid soft reference to UWorld"));
		return false;
	}

	bool bSuccess = false;
	LevelStreamingDynamic = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		GetWorld(),
		TargetLevel,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		bSuccess
	);

	GetWorld()->FlushLevelStreaming();

	if (bSuccess && LevelStreamingDynamic)
	{
		return true;
	}
	return false;
}

// Uncompleted
void UZHJZ_GameInstance::OnLevelLoaded()
{
	if (OnLevelChanged.IsBound())
	{
		OnLevelChanged.Broadcast(GetWorld());
	}
}

void UZHJZ_GameInstance::RefreshTime()
{
	FDateTime Now = FDateTime::Now();
	TimeValue = Now.GetHour() * 100 + Now.GetMinute();
}

int32 UZHJZ_GameInstance::GetNow()
{
	return TimeValue;
}
