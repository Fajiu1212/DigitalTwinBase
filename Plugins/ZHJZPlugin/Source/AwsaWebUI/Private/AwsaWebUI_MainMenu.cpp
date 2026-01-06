#include "AwsaWebUI_MainMenu.h"

#include "AwsaWebUI_SaveGame.h"
#include "AwsaWebUI_Subsystem.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "WebUI/Public/WebInterface.h"

bool UAwsaWebUI_MainMenu::Initialize()
{
	return Super::Initialize();
}

void UAwsaWebUI_MainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// WebInterface->LoadURL(TEXT(""));
	// LoadMainMenu();
	// FTimerHandle WebTimerHandle;
	// GetWorld()->GetTimerManager().SetTimer(WebTimerHandle, [this]()
	// {
	// 	LoadMainMenu();
	// }, 3.f, false);
}

void UAwsaWebUI_MainMenu::BeginDestroy()
{
	Super::BeginDestroy();
}

void UAwsaWebUI_MainMenu::OnReceiveWebMsg(const FName Name, FJsonLibraryValue Data, FWebInterfaceCallback Callback)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}
	UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (Subsystem)
	{
		FString Content = Data.GetString();
		Subsystem->BroadcastManager(Name.ToString(), Content);
	}
}

void UAwsaWebUI_MainMenu::CallWebFunc(const FString& Function, const FJsonLibraryValue& Data)
{
	UAutoParse::PrintLog_GameThread(
		FString::Printf(TEXT("CallWebFunc:::%s,Jsonvalue:::%s"), *Function, *Data.GetString()));

	WebInterface->Call(Function, Data);
}

void UAwsaWebUI_MainMenu::LoadMainMenu()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}
	
	UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!Subsystem)
	{
		return;
	}
	WebUI_Url = Subsystem->GetEnvUrl(
		FGameplayTag::RequestGameplayTag(Subsystem->CheckCondition(TEXT("bIsTestEnv"))
											 ? TEXT("Url.Client.MainUI.Test")
											 : TEXT("Url.Client.MainUI")));
	// WebUI_Url = Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Url.Client.MainUI")));
	WebUI_Token = Subsystem->GetToken();
	WebUI_AppCode = Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.AppCode")));
	WebUI_OrgId = Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.OrgId")));
	WebUI_ProjectCode = Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.ProjectCode")));
	
	FString InitVersionStr = Subsystem->GetEnvUrl(FGameplayTag::RequestGameplayTag(TEXT("Param.Version")));
	UAwsaWebUI_SaveGame* SaveInstance = nullptr;
	
	if (UGameplayStatics::DoesSaveGameExist("Version", 0))
	{
		SaveInstance = Cast<UAwsaWebUI_SaveGame>(UGameplayStatics::LoadGameFromSlot("Version", 0));
	
		if (SaveInstance)
		{
			TArray<FString> SaveParts, InitParts;
			SaveInstance->Version.ParseIntoArray(SaveParts,TEXT("."), true);
			InitVersionStr.ParseIntoArray(InitParts,TEXT("."), true);
	
			if (SaveParts.Num() > 1 && InitParts.Num() > 1 && SaveParts[0] == InitParts[0] && SaveParts[1] == InitParts[
				1])
			{
				WebUI_Version = SaveInstance->Version;
			}
			else
			{
				WebUI_Version = InitVersionStr;
				SaveInstance->Version = InitVersionStr;
				UGameplayStatics::SaveGameToSlot(SaveInstance, "Version", 0);
			}
		}
	}
	else
	{
		SaveInstance = Cast<UAwsaWebUI_SaveGame>(
			UGameplayStatics::CreateSaveGameObject(UAwsaWebUI_SaveGame::StaticClass())); 
		if (SaveInstance)
		{
			SaveInstance->Version = InitVersionStr;
			WebUI_Version = InitVersionStr;
			UGameplayStatics::SaveGameToSlot(SaveInstance, "Version", 0);
		}
	}
	
	Subsystem->OnCallWebFunc.AddDynamic(this, &UAwsaWebUI_MainMenu::CallWebFunc);
	Subsystem->OnCallDisableWebUI.AddDynamic(this, &UAwsaWebUI_MainMenu::SetDisableHoverPanel);
	
	FString AppendURL = FString::Printf(
		TEXT("%s&token=%s&appCode=%s&orgId=%s&version=%s&projectCode=%s"), *WebUI_Url, *WebUI_Token, *WebUI_AppCode,
		*WebUI_OrgId,
		*WebUI_Version, *WebUI_ProjectCode);
	// FString AppendURL = FString::Printf(
	// 	TEXT("%s&token=%s&orgId=%s"), *WebUI_Url, *WebUI_Token,
	// 	*WebUI_OrgId);
	UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("LoadURL:::%s"), *AppendURL));
	WebInterface->EnableIME();
	WebInterface->OnInterfaceEvent.Clear();
	WebInterface->LoadURL(AppendURL);
	WebInterface->OnInterfaceEvent.AddDynamic(this, &UAwsaWebUI_MainMenu::OnReceiveWebMsg);
	WebInterface->OnUrlChangedEvent.AddDynamic(this, &UAwsaWebUI_MainMenu::OnUrlChanged);

#pragma region LocalHtml
	// UGameInstance* GameInstance = GetGameInstance();
	// if (!GameInstance)
	// {
	// 	return;
	// }
	//
	// UAwsaWebUI_Subsystem* Subsystem = GameInstance->GetSubsystem<UAwsaWebUI_Subsystem>();
	// if (!Subsystem)
	// {
	// 	return;
	// }
	//
	// Subsystem->OnCallWebFunc.AddDynamic(this, &UAwsaWebUI_MainMenu::CallWebFunc);
	//
	// WebInterface->OnInterfaceEvent.Clear();
	// WebInterface->LoadFile(
	// 	TEXT(
	// 		"HTML/dist/index.html#/ZY250082/page"
	// 		"?token=eyJhbGciOiJIUzI1NiJ9.eyJzdGEiOjE3NjExODYxMjY1MzAsIm5hbWUiOiLmnInpl7TkupEiLCJ1c2VyTmFtZSI6ImF3c2EiLCJleHAiOjE3NjE3OTA5MjY1MzAsInVzZXJJZCI6MSwib3JnSWQiOjQzM30.kDtA_-WWmtQRayDq4GHXZTnK03A3g8EoHBQgxkc4aMU"
	// 		"&env=test&orgId=107"),
	// 	EWebInterfaceDirectory::Content);
	// WebInterface->OnInterfaceEvent.AddDynamic(this, &UAwsaWebUI_MainMenu::OnReceiveWebMsg);
	// WebInterface->OnUrlChangedEvent.AddDynamic(this, &UAwsaWebUI_MainMenu::OnUrlChanged);
#pragma endregion
}

FString UAwsaWebUI_MainMenu::GetScreenMode()
{
	EWindowMode::Type CurrentMode = GEngine->GameViewport->GetWindow()->GetWindowMode();
	FString ModeStr = CurrentMode == EWindowMode::Type::Windowed
		                  ? TEXT("Windowed")
		                  : (CurrentMode == EWindowMode::Type::Fullscreen
			                     ? TEXT("Fullscreen")
			                     : TEXT("WindowedFullscreen"));
	return ModeStr;
}

void UAwsaWebUI_MainMenu::SetDisableHoverPanel(bool bDisable)
{
	if (Image_DisableHoverPanel)
	{
		Image_DisableHoverPanel->SetVisibility(bDisable? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UAwsaWebUI_MainMenu::OnUrlChanged(const FText& URL)
{
	FString URLStr = URL.ToString();
	FString MainURL = FString::Printf(
		TEXT("%s&token=%s&appCode=%s&orgId=%s&version=%s&projectCode=%s"), *WebUI_Url, *WebUI_Token, *WebUI_AppCode,
		*WebUI_OrgId,
		*WebUI_Version, *WebUI_ProjectCode);
	if (URLStr == MainURL)
	{
		OnUrlChanged_BP();
	}
}
