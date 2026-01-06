#include "WebUI_Manager.h"

#include "AutoParse.h"
#include "Kismet/KismetSystemLibrary.h"

ULevel* UWebUI_Manager::GetLevel() const
{
	return GetTypedOuter<ULevel>();
}

void UWebUI_Manager::ReceiveFunction_Implementation(const FString& Name, const FString& Message)
{
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("ReceiveFunction is not implemented.Message:::%s"), *Message));
}

void UWebUI_Manager::ExitGame()
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

void UWebUI_Manager::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.AppendTags(WebTagContainer);
}

UWorld* UWebUI_Manager::GetWorld() const
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
