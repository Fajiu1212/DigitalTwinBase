#pragma once

#include "CoreMinimal.h"
#include "AutoParse.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebUI_Subsystem.generated.h"

class UWebUI_Manager;

UCLASS(Config=Toolkits)
class WEBWIDGET_API UWebUI_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UWebUI_Subsystem();
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//Token
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Toolkits|WebUI")
	FString GetToken()
	{
		return Token;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Toolkits|WebUI")
	FString GetToken_UrlAppend()
	{
		return TEXT("?token=") + Token;
	}

	//orgid
private:
	UPROPERTY(Config)
	FString ConfigOrgId;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Toolkits|WebUI")
	FString GetOrgId()
	{
		return ConfigOrgId;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Toolkits|WebUI")
	FString GetOrgId_UrlAppend()
	{
		return TEXT("&oigId=") + ConfigOrgId;
	}

private:
	FString Token = FString();
	UPROPERTY(config)
	FString ConfigTokenURL;
	UPROPERTY(Config)
	FString ConfigTokenVerb;
	UPROPERTY(config)
	FString ConfigTokenContent;
	void TokenRequest();
	UFUNCTION()
	void SaveTokenRequest(const FJsonStruct& JsonStruct, const FString&
	                      WebMsg, const FString& StructName);
	//Manager
	UPROPERTY()
	TObjectPtr<UWebUI_Manager> WebUIManager = nullptr;
	void CreateWebUIManager();

public:
	UFUNCTION(BlueprintCallable, Category = "Toolkits|WebUI")
	UWebUI_Manager* GetWebUIManager() const { return WebUIManager; }


	//Listeners
public:
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void RegisterListener(UObject* Listener);
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void UnregisterListener(UObject* Listener);

	void bBroadcastManager(const FString& Name, const FString& Message);
	UFUNCTION(BlueprintCallable, Category="Toolkits|WebUI")
	void BroadcastMessage(const FString& Name, const FString& Message, const FGameplayTag& RequiredTag);

private:
	TArray<TWeakObjectPtr<UObject>> Listeners;
};
