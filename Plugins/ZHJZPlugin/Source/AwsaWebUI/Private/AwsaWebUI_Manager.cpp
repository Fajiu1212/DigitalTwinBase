#include "AwsaWebUI_Manager.h"

#include "AutoParse.h"
#include "AwsaWebUI_SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


ULevel* UAwsaWebUI_Manager::GetLevel() const
{
	return GetTypedOuter<ULevel>();
}

void UAwsaWebUI_Manager::InitManager_Implementation()
{
}

void UAwsaWebUI_Manager::ReceiveFunction_Implementation(const FString& Name, const FString& Message)
{
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("ReceiveFunction is not implemented.Message:::%s"), *Message));
}

void UAwsaWebUI_Manager::ExitGame()
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			UKismetSystemLibrary::QuitGame(
				World,
				PlayerController,
				EQuitPreference::Quit,
				false
			);
		}
	}
}

void UAwsaWebUI_Manager::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(WebTagContainer);
}

UWorld* UAwsaWebUI_Manager::GetWorld() const
{
	if (!HasAnyFlags(RF_ClassDefaultObject) && ensureMsgf(GetOuter(),
	                                                      TEXT("Actor: %s has a null OuterPrivate in AActor::GetWorld()"
	                                                      ), *GetFullName())
		&& !GetOuter()->HasAnyFlags(RF_BeginDestroyed) && !GetOuter()->IsUnreachable())
	{
		if (UWorld* World = GetOuter()->GetWorld())
		{
			return World;
		}
	}
	return nullptr;
}

void UAwsaWebUI_Manager::UpdateVersion(FString NewVersion)
{
	UAwsaWebUI_SaveGame* SaveInstance;
	if (UGameplayStatics::DoesSaveGameExist("Version", 0))
	{
		SaveInstance = Cast<UAwsaWebUI_SaveGame>(UGameplayStatics::LoadGameFromSlot("Version", 0));
		UAutoParse::PrintLog_GameThread(TEXT("SaveGameLoaded"));
		SaveInstance->Version = NewVersion;
		UGameplayStatics::SaveGameToSlot(SaveInstance, "Version", 0);
	}
}

void UAwsaWebUI_Manager::GetOldVersion(FString& OldVersion)
{
	UAwsaWebUI_SaveGame* SaveInstance;
	if (UGameplayStatics::DoesSaveGameExist("Version", 0))
	{
		SaveInstance = Cast<UAwsaWebUI_SaveGame>(UGameplayStatics::LoadGameFromSlot("Version", 0));
		OldVersion = SaveInstance->Version;
	}
}

void UAwsaWebUI_Manager::Interpolator(float Curr, float Target, float Duration, float Step)
{
	UWorld* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return;
	}
	struct FLocalInterpolator
	{
		float Curr, Target, Duration, Step, Elapsed, DeltaPerStep;
		int32 TotalSteps;

		FLocalInterpolator(float InStart, float InTarget, float InDuration, float InStep)
			: Curr(InStart), Target(InTarget), Duration(InDuration), Step(InStep), Elapsed(0.0f)
		{
			Duration = FMath::Max(InDuration, KINDA_SMALL_NUMBER);
			Step = FMath::Max(InStep, KINDA_SMALL_NUMBER);
			TotalSteps = FMath::Max(1, FMath::RoundToInt(Duration / Step));
			DeltaPerStep = (Target - Curr) / TotalSteps;
		}

		bool Next(float& OutValue)
		{
			if (Elapsed >= Duration)
			{
				OutValue = Target;
				return false;
			}
			Curr += DeltaPerStep;
			Elapsed += Step;
			if ((DeltaPerStep > 0.0f && Curr > Target) || (DeltaPerStep < 0.0f && Curr < Target))
			{
				Curr = Target;
			}
			OutValue = Curr;
			return Elapsed < Duration;
		}
	};
	
	TSharedPtr<FLocalInterpolator> Interp = MakeShared<FLocalInterpolator>(Curr, Target, Duration, Step);
	FTimerDelegate TimerDel;
	TimerDel.BindLambda([Interp, WorldContext,this]()
	{
		float Value;
		if (!Interp->Next(Value))
		{
			UE_LOG(LogTemp, Log, TEXT("Final Value: %f"), Value);
			WorldContext->GetTimerManager().ClearTimer(InterpTimerHandle);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Value: %f"), Value);
		}
	});
	WorldContext->GetTimerManager().SetTimer(InterpTimerHandle, TimerDel, 0.02, true);
}
