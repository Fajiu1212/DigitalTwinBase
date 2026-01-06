#include "WebUI_Subsystem.h"

#include "GameplayTagAssetInterface.h"
#include "WebUI_Interface.h"
#include "WebUI_Manager.h"

UWebUI_Subsystem::UWebUI_Subsystem()
{
	CreateWebUIManager();
}

bool UWebUI_Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return false;
}

void UWebUI_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TokenRequest();
}

void UWebUI_Subsystem::Deinitialize()
{
	WebUIManager = nullptr;
	Super::Deinitialize();
}

void UWebUI_Subsystem::TokenRequest()
{
	if (ConfigTokenURL != TEXT("") && ConfigTokenVerb != TEXT("") && ConfigTokenContent != TEXT(""))
	{
		ConfigTokenURL = TEXT("https://smart.ilabx.cn/user/Login/getToken");
		ConfigTokenVerb = TEXT("POST");
		ConfigTokenContent = TEXT(
			"{\"orgId\" : \"1698\",  \"password\" : \"fb7d5e8e7bbf2154fd88bcfe97c5b474\",  \"userName\" : \"awsa\"}");
	}

	UAutoParse* Instance = UAutoParse::AutoHttpRequest(ConfigTokenURL, ConfigTokenContent, ConfigTokenVerb,
	                                                   TMap<FString, FString>(), FString(), ERequestType::HttpOnly);
	Instance->OnSuccess.AddDynamic(this, &UWebUI_Subsystem::SaveTokenRequest);
}

void UWebUI_Subsystem::SaveTokenRequest(const FJsonStruct& JsonStruct, const FString& WebMsg, const FString& StructName)
{
	FAutoParseJsonObject JsonMsg;
	UAutoParse::GetJsonObjectFromJsonStr(WebMsg, JsonMsg);
	FString StrToken = FString();
	UAutoParse::GetStrValueFormObject(TEXT("msg"), JsonMsg, StrToken);
	Token = StrToken;
}

void UWebUI_Subsystem::CreateWebUIManager()
{
	ConstructorHelpers::FClassFinder<UWebUI_Manager> BP_WebUIManagerClassFinder(
		TEXT("/Script/Engine.Blueprint'/Game/Blueprints/BP_Public/Manager/BP_WebUIManager.BP_WebUIManager_C'"));
	
	if (BP_WebUIManagerClassFinder.Succeeded() && BP_WebUIManagerClassFinder.Class)
	{
		WebUIManager = NewObject<UWebUI_Manager>(this, BP_WebUIManagerClassFinder.Class, TEXT("BP_WebUIManager"));
	}
	else
	{
		WebUIManager = NewObject<UWebUI_Manager>(this, TEXT("WebUIManager"));
	}
}

void UWebUI_Subsystem::RegisterListener(UObject* Listener)
{
	if (Listener && Listener->Implements<UWebWidget_Interface>())
	{
		Listeners.AddUnique(Listener);
	}
}

void UWebUI_Subsystem::UnregisterListener(UObject* Listener)
{
	if (Listener && Listeners.Contains(Listener))
	{
		Listeners.Remove(Listener);
	}
}

void UWebUI_Subsystem::bBroadcastManager(const FString& Name, const FString& Message)
{
	if (WebUIManager)
	{
		IWebWidget_Interface::Execute_ReceiveFunction(WebUIManager.Get(),Name,Message);
	}
}

void UWebUI_Subsystem::BroadcastMessage(const FString& Name, const FString& Message, const FGameplayTag& RequiredTag)
{
	for (auto& Listener : Listeners)
	{
		if (Listener.IsValid())
		{
			if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Listener.Get()))
			{
				FGameplayTagContainer ListenerTags;
				TagInterface->GetOwnedGameplayTags(ListenerTags);
				if (ListenerTags.HasTag(RequiredTag))
				{
					IWebWidget_Interface::Execute_ReceiveFunction(Listener.Get(), Name, Message);
				}
			}
		}
	}
}
