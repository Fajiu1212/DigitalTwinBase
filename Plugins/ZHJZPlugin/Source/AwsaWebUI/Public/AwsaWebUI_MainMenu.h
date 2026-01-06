#pragma once

#include "CoreMinimal.h"
#include "JsonLibraryValue.h"
#include "Blueprint/UserWidget.h"
#include "WebUI/Public/WebInterfaceCallback.h"
#include "AwsaWebUI_MainMenu.generated.h"

class UImage;
class UWebInterface;


UCLASS(Config=Toolkits)
class AWSAWEBUI_API UAwsaWebUI_MainMenu : public UUserWidget
{
	GENERATED_BODY()

	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void BeginDestroy() override;

public:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UWebInterface* WebInterface;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UImage* Image_DisableHoverPanel;

	UFUNCTION()
	void OnUrlChanged(const FText& URL);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category="AwsaWebUI|MainMenu")
	void OnUrlChanged_BP();
	UFUNCTION(BlueprintCallable, Category="AwsaWebUI|MainMenu")
	void OnReceiveWebMsg(const FName Name, FJsonLibraryValue Data, FWebInterfaceCallback Callback);

	FString WebUI_Url = TEXT("");
	FString WebUI_Token = TEXT("");
	FString WebUI_OrgId = TEXT("");	
	FString WebUI_AppCode = TEXT("");
	FString WebUI_Version = TEXT("");
	FString WebUI_ProjectCode = TEXT("");

	UFUNCTION()
	void CallWebFunc(const FString& Function, const FJsonLibraryValue& Data);
	//Temp sln.
	void LoadMainMenu();

	UFUNCTION(BlueprintCallable,Category="AwsaWebUI|MainMenu")
	FString GetScreenMode();

	UFUNCTION()
	void SetDisableHoverPanel(bool bDisable);
};
