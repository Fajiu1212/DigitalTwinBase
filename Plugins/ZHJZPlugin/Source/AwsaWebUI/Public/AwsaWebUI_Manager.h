#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "AwsaWebUI_Interface.h"
#include "UObject/Object.h"
#include "AwsaWebUI_Manager.generated.h"

UCLASS(Blueprintable)
class AWSAWEBUI_API UAwsaWebUI_Manager : public UObject, public IAwsaWebUI_Interface, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	virtual class UWorld* GetWorld() const override;
	class ULevel* GetLevel() const;

	// Init
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "AwsWebUI")
	void InitManager();
	// Gameplay tag
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AwsaWebUI|GameplayTag")
	FGameplayTagContainer WebTagContainer;
	
	// Web Interface
	virtual void ReceiveFunction_Implementation(const FString& Name, const FString& Message) override;

	// Default func
	UFUNCTION(BlueprintCallable, Category = "AwsaWebUI|Interface")
	void ExitGame();
	
	UFUNCTION(BlueprintCallable, Category="AwsaWebUI|MainMenu")
	void UpdateVersion(FString NewVersion);
	UFUNCTION(BlueprintCallable, Category="AwsaWebUI|MainMenu")
	void GetOldVersion(FString& OldVersion);

	UFUNCTION(BlueprintCallable, Category = "AwsaWebUI|MainMenu")
	void Interpolator(float Curr, float Target, float Duration, float Step);
	FTimerHandle InterpTimerHandle;
};
