// Fill out your copyright notice in the Description page of Project Settings.


#include "AwsaWebUI_ActorDashboard.h"

#include "AwsaWebUI_Subsystem.h"
#include "WebUI/Public/WebInterface.h"

void UAwsaWebUI_ActorDashboard::NativeConstruct()
{
	Super::NativeConstruct();
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>();
			if (Subsystem)
			{
				AppendToken = Subsystem->GetToken_UrlAppend();
				AppendOrgId = Subsystem->GetOrgId_UrlAppend();
			}
		}
	}
	FString AppendURL = DashboardWebUI_Url + AppendToken + AppendOrgId + ConvertTMapToQueryString(OtherParamMap);
	
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("LoadURL:::%s"), *AppendURL));
	WebInterface->EnableIME();
	WebInterface->LoadURL(AppendURL);
	// check is bound
	if (!WebInterface->OnInterfaceEvent.IsBound())
	{
		WebInterface->OnInterfaceEvent.AddDynamic(this, &UAwsaWebUI_ActorDashboard::OnReceiveWebMsg);
	}
}





void UAwsaWebUI_ActorDashboard::OnReceiveWebMsg(const FName Name, FJsonLibraryValue Data, FWebInterfaceCallback Callback)
{
	if (!IsValid(this)) return;
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>();
			if (Subsystem)
			{
				FString Content = Data.GetString();
				Subsystem->BroadcastManager(Name.ToString(), Content);
			}
		}
	}

}


FString UAwsaWebUI_ActorDashboard::ConvertTMapToQueryString(const TMap<FString, FString>& InMap) {
	FString Result;
	for (const TPair<FString, FString>& Pair : InMap) {
		if (!Result.IsEmpty()) Result += "&";
		Result += FString::Printf(TEXT("%s=%s"), *Pair.Key, *Pair.Value);
	}
	return Result;
}