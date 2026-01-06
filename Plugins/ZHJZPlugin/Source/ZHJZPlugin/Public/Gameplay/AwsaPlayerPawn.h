#pragma once

#include "CoreMinimal.h"
#include "PlacePreviewComponent.h"
#include "GameFramework/Pawn.h"
#include "AwsaPlayerPawn.generated.h"

class USphereComponent;
class UFloatingPawnMovement;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;

UENUM(BlueprintType)
enum class EControlType :uint8
{
	Default,
	Fpp,
	Tpp,
	Observer,
	Disable
};

UENUM(BlueprintType)
enum class EViewMode :uint8
{
	ThirdPerson,
	FirstPerson
};

USTRUCT(Blueprintable)
struct FExplosionTransform
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	FVector Location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	FRotator Rotation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	float CameraArmLength;
};

UCLASS()
class ZHJZPLUGIN_API AAwsaPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AAwsaPlayerPawn();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Destroyed() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category="Gameplay|Character")
	static AAwsaPlayerPawn* GetAwsaPlayerPawn();

protected:
	// View mode
	UPROPERTY(BlueprintReadWrite, Category="Gameplay|Character")
	EViewMode CurrViewMode;
	FVector FirstPersonCameraOffset = {};
	// Saved tpp transform info
	float SavedSpringArmLength = 0.f;
	FVector SavedPawnLocation = FVector::ZeroVector;
	FRotator SavedControlRotation = FRotator::ZeroRotator;
	// Mouse
	void CheckMouseHover();
	void CheckMouseHover_Component();
	
	void OnLeftMouseClicked(const FInputActionValue& Value);
	void OnLeftMouseClicked_Component(const FInputActionValue& Value);
	void OnRightMouseClicked(const FInputActionValue& Value);

	bool GetActorUnderCursor(FHitResult& OutHitResult) const;

	void NotifyMouseOverlay(AActor* Actor);
	void NotifyMouseLeave(AActor* Actor);
	void NotifyMouseClick(AActor* Actor, bool bIsLeftClick);

	void NotifyMouseOverlay_Component(UMeshComponent* Component);
	void NotifyMouseLeave_Component(UMeshComponent* Component);
	void NotifyMouseClick_Component(UMeshComponent* Component);

	// PlacePreview
	void OnPlacePreviewAction(const FInputActionValue& Value);
	UFUNCTION(BlueprintCallable, Category="Gameplay|Character")
	void ExitPlacePreview();

	// Upload Test Environment Mode
	void OnUploadTestEnvModeAction(const FInputActionValue& Value);
	//gm
	void OnGKeyPressed(const FInputActionValue& Value);
	void OnGKeyReleased(const FInputActionValue& Value);
	void OnMKeyPressed(const FInputActionValue& Value);
	void OnMKeyReleased(const FInputActionValue& Value);

	void RecordMousePosition();

public:
	UFUNCTION(BlueprintCallable, Category="Gameplay|Character")
	void SwitchViewMode(EViewMode TargetViewMode);
	UFUNCTION(BlueprintCallable, Category = "PlayerPawn|Camera")
	void MoveCameraToTarget(FVector TargetLocation, FRotator TargetRotation, float TargetArmLength, float Duration);
	UFUNCTION(BlueprintCallable, Category = "PlayerPawn|Camera")
	void MovePawnToTarget(FVector TargetLocation, FRotator TargetRotation, float Duration);

public:
	//PlacePreview Component
	bool bIsPlacePreviewActive = false;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay|Character|PlacePreview",
		meta = (AllowPrivateAccess = "true"))
	UPlacePreviewComponent* PlacePreviewComponent;
	UFUNCTION()
	void OnPlacePreviewMode(bool bPlacePreviewMode);

	//Camera
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	USphereComponent* FirstPersonCollisionSphere;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	float TargetArmLength = 2000.f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	float MaxTargetArmLength = 10000.f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	float MinTargetArmLength = 0.f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	float ScaleFactor = 1.f;
	float ReferenceArmLength = 4600.f;

protected:
	// HoveredActor
	UPROPERTY(Transient)
	AActor* CurrentHoveredActor;
	UPROPERTY(Transient)
	AActor* LastHoveredActor;
	
	// HoveredComponent
	UPROPERTY(Transient)
	UMeshComponent* CurrentHoveredComponent;
	UPROPERTY(Transient)
	UMeshComponent* LastHoveredComponent;
	
	//Base default actions
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LeftMouseClickAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* LeftMouseHoldAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* RightMouseClickAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* RightMouseHoldAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* PlacePreviewAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* UploadTestEnvModeAction;


	bool bGKeyPressed;
	bool bMKeyPressed;
	bool bComboActive;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* GKeyAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MKeyAction;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UPositionDisplayUI> PositionWidgetClass;

	UPROPERTY()
	UPositionDisplayUI* PositionWidget;

	void ShowPositionInUI(const FVector& Position);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	float CameraMoveSpeed = 14000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	float ScaleSpeed = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
	float MinScaleSpeed = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Camera Movement")
	float MaxScaleSpeed = 20000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fpp Movement")
	float FppMoveSpeed = 6000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fpp Movement")
	float FppLookSensitivity = 3.f;

public:
	UFUNCTION(Blueprintable, Category="Camera Movement")
	void SetCameraMoveSpeed(float TargetMoveSpeed);

private:
	//Movement
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	UFloatingPawnMovement* MovementComponent;

#pragma region Input actions
	//Mapping context
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character|InputAction",
		meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character|InputAction",
		meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* TppMappingContext;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character|InputAction",
		meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* FppMappingContext;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character|InputAction",
		meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* ObserverMappingContext;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character|InputAction",
		meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DisableMappingContext;

	//Switch IMC
	EControlType CurrControlType = EControlType::Tpp;
	UPROPERTY()
	UInputMappingContext* CurrMappingContext;
	UFUNCTION(BlueprintCallable, Category = "Gameplay|Character|InputAction")
	void SwitchMappingContext(EControlType TargetType);

	//Controller input actions
	// TPP
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	UInputAction* CameraMoveAction;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	UInputAction* CameraRotateAction;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	UInputAction* CameraScalingAction;
	// FPP
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	UInputAction* FPPMovementAction;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Gameplay|Character", meta = (AllowPrivateAccess = "true"))
	UInputAction* FPPLookAtAction;

	// TPP
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Rotator(const FInputActionValue& Value);
	UFUNCTION()
	void Scaling(const FInputActionValue& Value);
	// FPP
	UFUNCTION()
	void FppMove(const FInputActionValue& Value);
	UFUNCTION()
	void FppLook(const FInputActionValue& Value);
#pragma endregion

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera",
		meta = (ClampMin = 0.0, AllowPrivateAccess = "true"))
	float RotationSpeed = 50.0f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera",
		meta = (ClampMin = -89.0, ClampMax = -80.0, AllowPrivateAccess = "true"))
	float MinPitch = -85.f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Camera",
		meta = (ClampMin = -20.0, ClampMax = -1.0, AllowPrivateAccess = "true"))
	float MaxPitch = -10.f;
	float CurrentPitch = -45.f;

	FVector TargetCameraLocation;
	float TargetCameraArmLength;
	float MoveToTargetDuration;
	float CurrentMoveTime;
	bool bIsMovingToTarget;

	FVector StartLocation;
	float StartArmLength;

	FRotator StartRotation;
	FRotator TargetCameraRotation;

	UPROPERTY()
	float CurrentMoveSpeed = 1000;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "MoveSpeed", meta = (AllowPrivateAccess = "true"))
	float MinMoveSpeed = 50.f;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "MoveSpeed", meta = (AllowPrivateAccess = "true"))
	float MaxMoveSpeed = 15000.f;
	UFUNCTION()
	void UpdateMovementSpeedByCameraHeight();

	// Init camera moving

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	FVector InitCameraBeginLocation;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	FRotator InitCameraBeginRotation;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	float InitCameraBeginArmLength;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	float InitCameraBeginDuration;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	FVector InitCameraEndLocation;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	FRotator InitCameraEndRotation;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	float InitCameraEndArmLength;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="InitCamera", meta = (AllowPrivateAccess = "true"))
	float InitCameraEndDuration;

public:
	UFUNCTION(BlueprintCallable, Category="InitCamera")
	void InitCameraMoving();

	//OnExplosionAnimation
	UFUNCTION()
	void OnExplosionAnimation(const FString& OwnerName, bool bIsMultiple);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExplosionAnimation")
	TMap<FString, FExplosionTransform> CameraTransformMap;

	// On module switched
	UFUNCTION()
	void OnModuleSwitched(const FString& NewModuleName, const FString& InExtraParam);

	// 隧道剖切
	UFUNCTION()
	void OnSectionCuttingSelected(const FString& TargetTunnelName);
	// Temp NoDeviceMonitoring
	UFUNCTION(BlueprintImplementableEvent,Category="Gameplay|Character")
	void MoveToNoDeviceMonitoring(const FString& DeviceParam);
};
