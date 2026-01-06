#pragma once

#include "CoreMinimal.h"
#include "BeamGeneratorSubsystem.h"
#include "Func/ZHJZ_Interface.h"
#include "GameFramework/Actor.h"
#include "BeamActorBase.generated.h"

class UBeamWidgetBase;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EModelType : uint8
{
	WithTemplate,
	NoTemplate
};

UENUM(BlueprintType)
enum class EAiType : uint8
{
	AiUnsupported,
	AiBeam,
	AiPedestal
};

UENUM(BlueprintType)
enum class EBeamActorState : uint8
{
	Default,
	Highlight,
	HighlightShowPoi,
	ShowMenu,
	HighlightShowMenu
};

UCLASS()
class ZHJZPLUGIN_API ABeamActorBase : public AActor, public IZHJZ_Interface
{
	GENERATED_BODY()

public:
	ABeamActorBase();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ZHJZ_Interface implementation
	virtual void MouseOverlay_Implementation() override;
	virtual void MouseLeave_Implementation() override;
	virtual void MouseClick_Implementation(bool isBool) override;

	// Components
	UPROPERTY(BlueprintReadWrite, Category="Beam|BeamActorBase")
	UStaticMeshComponent* MeshComponent;
	UPROPERTY(BlueprintReadWrite, Category="Beam|BeamActorBase")
	UWidgetComponent* PoiWidgetComponent;

	// Beam struct
	UPROPERTY(BlueprintReadWrite, Category="Beam|BeamActorBase")
	FBeamStruct BeamStruct;

	// Beam state
	EBeamActorState BeamState = EBeamActorState::Default;
	void OnBeamStateChanged(EBeamActorState TargetState);
	void HighlightEffect(bool bIsHighlight);
	void TogglePoi(bool bIsPoiVisible);
	void ResetBeamActor();
	void InitBeamActor();
	UFUNCTION(BlueprintImplementableEvent, Category="Beam|BeamActorBase")
	void HightlightSkeletalMesh(bool bIsHighlight);

	// InitBeamActorByData
	void SetBeamStruct(const FBeamStruct& NewBeamStruct);

	// bShow
	bool bShowPedestal = false;
public:
	UFUNCTION(BlueprintCallable, Category="Beam|BeamActorBase")
	FBeamStruct GetBeamStruct() { return BeamStruct; }

	UFUNCTION(BlueprintCallable, Category="Beam|BeamActorBase")
	EAiType GetBeamType() { return BeamType; }

	UFUNCTION(BlueprintCallable, Category="Beam|BeamActorBase")
	void ChangeBeamState(EBeamActorState NewState);

	UFUNCTION()
	bool GetPedestalVisibility() const { return bShowPedestal; }
	UFUNCTION()
	void ToggleVisibility(bool bIsVisible);


	UFUNCTION(BlueprintNativeEvent, Category="Beam|BeamActorBase")
	void ChangeAiMode(bool bIsAiMode);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Beam|BeamActorBase")
	void MoveToBeam();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Beam|BeamActorBase")
	void CallWebCenterPopup();

	UFUNCTION(BlueprintNativeEvent, Category="Beam|BeamActorBase")
	void OnBeamActorClicked();

	// BP params
	// Type
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings|BeamActorBase",
		meta = (DisplayPriority = 0, DisplayName = "BeamActorBaseSettings"))
	EAiType BeamType = EAiType::AiUnsupported;
	// Normal
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|BeamActorBase",
		meta = (DisplayPriority = 0, DisplayName = "TargetArmLength"))
	float MovementTargetArmLength = 2000.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|BeamActorBase",
		meta = (DisplayPriority = 0, DisplayName = "MovementDuration"))
	float MovementDuration = 1.2f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|BeamActorBase",
		meta = (DisplayPriority = 0, DisplayName = "WidgetDrawSize"))
	FVector2D WidgetDrawSize = FVector2D(260, 148);
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|BeamActorBase",
		meta = (DisplayPriority = 0, DisplayName = "WidgetOffset"))
	FVector WidgetOffset = FVector(0, 0, 300);
	// Poi Widget Class
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|BeamActorBase")
	TSubclassOf<UBeamWidgetBase> PoiWidgetClass = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings|BeamActorBase",
		meta = (DisplayPriority = 0, DisplayName = "UnbindWidgetClass", EditCondition = "BeamType == EAiType::AiBeam",
			EditConditionHides))
	TSubclassOf<UBeamWidgetBase> PoiWidgetClass_Unbind = nullptr;

	// Temp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings|BeamActorBase")
	TSoftObjectPtr<UStaticMesh> BeamMesh;
	void UpdateMesh();

	// Update from request beam struct
	void UpdateFromBeamStruct(const FBeamStruct& NewBeamStruct);
};
