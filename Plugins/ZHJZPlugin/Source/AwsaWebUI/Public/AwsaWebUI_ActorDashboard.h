#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JsonLibraryValue.h"
#include "WebUI/Public/WebInterfaceCallback.h"
#include "AwsaWebUI_ActorDashboard.generated.h"

class UWebInterface;

UCLASS()
class AWSAWEBUI_API UAwsaWebUI_ActorDashboard : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
		UWebInterface* WebInterface;


	UFUNCTION(BlueprintCallable, Category = "AwsaWebUI|ActorDashboard")
		void OnReceiveWebMsg(const FName Name, FJsonLibraryValue Data, FWebInterfaceCallback Callback);


	UFUNCTION(BlueprintCallable)
		FString ConvertTMapToQueryString(const TMap<FString, FString>& InMap);

	   
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Url"))
	  FString DashboardWebUI_Url = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Title"))
	  FString DashboardTitle = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings",meta = (DisplayName = "Other Param"))
	  TMap<FString,FString> OtherParamMap;
	
	FString AppendToken = FString();
	FString AppendOrgId = FString();
};
