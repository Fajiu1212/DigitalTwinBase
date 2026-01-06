#pragma once

#include "CoreMinimal.h"
#include "Func/ZHJZ_Interface.h"
#include "GameFramework/Actor.h"
#include "BaseInTunnelActor.generated.h"

class AInTunnelManager;

UCLASS()
class ZHJZPLUGIN_API ABaseInTunnelActor : public AActor, public IZHJZ_Interface
{
	GENERATED_BODY()

public:
	ABaseInTunnelActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void MouseClick_Implementation(bool isBool) override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintReadWrite, Category = "BaseInTunnelActor")
	AInTunnelManager* InTunnelManager;
	void SetInTunnelManger(AInTunnelManager* BelongInTunnelManager);
	void UpdateInTunnelInfo();
	
	UFUNCTION(BlueprintImplementableEvent, Category="BaseInTunnelActor")
	void OnPoiClicked();
};
