 #pragma once

 #include "CoreMinimal.h"
 #include "JsonLibraryValue.h"
 #include "Blueprint/UserWidget.h"
 #include "WebUI/Public/WebInterfaceCallback.h"
 #include "WebUI_MainMenu.generated.h"

 class UWebInterface;


 UCLASS(Config=Toolkits)
 class WEBWIDGET_API UWebUI_MainMenu : public UUserWidget
 {
 	GENERATED_BODY()

 	virtual bool Initialize() override;
 	virtual void NativeConstruct() override;
 	virtual void BeginDestroy() override;

 public:
 	 UPROPERTY(BlueprintReadWrite,meta=(BindWidget))
 	 UWebInterface* WebInterface;
 	UFUNCTION()
 	void OnUrlChanged(const FText& URL);
 	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable,Category="Toolkits|WebUI")
 	void OnUrlChaned_BP();
 	UFUNCTION(BlueprintCallable,Category="Toolkits|WebUI")
 	void OnReceiveWebMsg(const FName Name, FJsonLibraryValue Data, FWebInterfaceCallback Callback);

	UPROPERTY(Config)
 	FString WebUI_Url = FString();
 	FString AppendToken = FString();
 	FString AppendOrgId = FString();
 };