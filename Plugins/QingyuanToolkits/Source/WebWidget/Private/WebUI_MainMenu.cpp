#include "WebUI_MainMenu.h"

#include "WebUI_GameplayTag.h"
#include "WebUI_Subsystem.h"
#include "WebUI/Public/WebInterface.h"

bool UWebUI_MainMenu::Initialize()
{
	return Super::Initialize();
}

void UWebUI_MainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			UWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UWebUI_Subsystem>();
			if (Subsystem)
			{
				AppendToken = Subsystem->GetToken_UrlAppend();
				AppendOrgId = Subsystem->GetOrgId_UrlAppend();
			}
		}
	}
	FString AppendURL = WebUI_Url + AppendToken + AppendOrgId;
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("LoadURL:::%s"), *AppendURL));
	WebInterface->LoadURL(AppendURL);
	WebInterface->OnInterfaceEvent.AddDynamic(this, &UWebUI_MainMenu::OnReceiveWebMsg);
	WebInterface->OnUrlChangedEvent.AddDynamic(this, &UWebUI_MainMenu::OnUrlChanged);
}

void UWebUI_MainMenu::BeginDestroy()
{
	Super::BeginDestroy();
}

void UWebUI_MainMenu::OnReceiveWebMsg(const FName Name, FJsonLibraryValue Data, FWebInterfaceCallback Callback)
{
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			UWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UWebUI_Subsystem>();
			if (Subsystem)
			{
				FString Content = Data.GetString();
				Subsystem->bBroadcastManager(Name.ToString(), Content);
			}
		}
	}
}

void UWebUI_MainMenu::OnUrlChanged(const FText& URL)
{
	FString URLStr = URL.ToString();
	FString MainURL = FString::Printf(TEXT("http://%s%s%s"), *WebUI_Url, *AppendToken, *AppendOrgId);
	if (URLStr == MainURL)
	{
		OnUrlChaned_BP();
	}
}
