#include "Gameplay/AwsaPlayerPawn.h"

#include "AutoParse.h"
#include "AwsaWebUI_Subsystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "JsonLibraryValue.h"
#include "Actor/EquipActorSubsystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Gameplay/AwsaPlayerController.h"

#include "Func/ZHJZ_Interface.h"
#include "UI/PositionDisplayUI.h"

AAwsaPlayerPawn::AAwsaPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	FirstPersonCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("FPPCollisionSphere"));
	RootComponent = FirstPersonCollisionSphere;
	FirstPersonCollisionSphere->SetSphereRadius(64.0f); 
	FirstPersonCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonCollisionSphere->SetCollisionObjectType(ECC_Pawn);
	FirstPersonCollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	FirstPersonCollisionSphere->SetGenerateOverlapEvents(true);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;
	MovementComponent->Acceleration = 5000000.f;
	MovementComponent->Deceleration = 8000000.f;
	MovementComponent->TurningBoost = 8.f;

	

	MovementComponent->UpdatedComponent = FirstPersonCollisionSphere;
	
	PlacePreviewComponent = CreateDefaultSubobject<UPlacePreviewComponent>(TEXT("PlacePreviewComponent"));
	//init
	CurrentHoveredActor = nullptr;
	LastHoveredActor = nullptr;

	bGKeyPressed = false;
	bMKeyPressed = false;
	bComboActive = false;
}

void AAwsaPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	ReferenceArmLength = (MaxTargetArmLength + MinTargetArmLength) / 2.f;
	MovementComponent->MaxSpeed = CameraMoveSpeed;
	// if (Controller)
	// { 
	// 	Controller->SetControlRotation(FRotator(-45, 0, 0));
	// }
	MoveCameraToTarget(InitCameraBeginLocation,InitCameraBeginRotation,InitCameraBeginArmLength,InitCameraBeginDuration);
	if (UEquipActorSubsystem* EquipActorSubsystem = GetGameInstance()->GetSubsystem<UEquipActorSubsystem>())
	{
		EquipActorSubsystem->OnPlacePreviewMode.AddDynamic(this, &AAwsaPlayerPawn::OnPlacePreviewMode);
	}
	SetActorEnableCollision(false);

	if (UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		WebUI_Subsystem->OnExplosionAnimation.AddDynamic(this, &AAwsaPlayerPawn::OnExplosionAnimation);
		WebUI_Subsystem->ModuleSwitched.AddDynamic(this, &AAwsaPlayerPawn::OnModuleSwitched);
		WebUI_Subsystem->OnSectionCuttingSelected.AddDynamic(this, &AAwsaPlayerPawn::OnSectionCuttingSelected);
	}
}

void AAwsaPlayerPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAwsaPlayerPawn::Destroyed()
{
	Super::Destroyed();
}

void AAwsaPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckMouseHover();
	CheckMouseHover_Component();
	if (bIsMovingToTarget)
	{
		CurrentMoveTime += DeltaTime;
		float Alpha = FMath::Clamp(CurrentMoveTime / MoveToTargetDuration, 0.0f, 1.0f);
		Alpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2);

		FVector NewLocation = FMath::Lerp(StartLocation, TargetCameraLocation, Alpha);
		if (Controller)
		{
			FRotator NewRotation = FMath::Lerp(StartRotation, TargetCameraRotation, Alpha);
			Controller->SetControlRotation(NewRotation);
		}
		SetActorLocation(NewLocation);

		if (CurrControlType == EControlType::Tpp)
		{
			float NewArmLength = FMath::Lerp(StartArmLength, TargetCameraArmLength, Alpha);
			SpringArm->TargetArmLength = NewArmLength;
			TargetArmLength = NewArmLength;
		}

		if (CurrentMoveTime >= MoveToTargetDuration)
		{
			bIsMovingToTarget = false;
			SetActorLocation(TargetCameraLocation);
			if (CurrControlType == EControlType::Tpp)
			{
				SpringArm->TargetArmLength = TargetCameraArmLength;
				TargetArmLength = TargetCameraArmLength;
				ScaleFactor = FMath::Max(SpringArm->TargetArmLength / ReferenceArmLength, 1);
			}
			if (Controller)
			{
				Controller->SetControlRotation(TargetCameraRotation);
			}
		}
	}

	bComboActive = bGKeyPressed && bMKeyPressed;
}

void AAwsaPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<AAwsaPlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			Subsystem->AddMappingContext(TppMappingContext, 1);
			CurrMappingContext = TppMappingContext;
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// TPP
		// Moving
		EnhancedInputComponent->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::Move);
		// Rotator
		EnhancedInputComponent->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::Rotator);
		// Scaling
		EnhancedInputComponent->BindAction(CameraScalingAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::Scaling);
		// FPP
		// Moving
		EnhancedInputComponent->BindAction(FPPMovementAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::FppMove);
		// Look
		EnhancedInputComponent->BindAction(FPPLookAtAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::FppLook);
		//mouse clicked
		EnhancedInputComponent->BindAction(LeftMouseClickAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::OnLeftMouseClicked);
		EnhancedInputComponent->BindAction(RightMouseClickAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::OnRightMouseClicked);

		//Place Preview
		EnhancedInputComponent->BindAction(PlacePreviewAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::OnPlacePreviewAction);

		//Upload test env. mode
		EnhancedInputComponent->BindAction(UploadTestEnvModeAction, ETriggerEvent::Triggered, this,
		                                   &AAwsaPlayerPawn::OnUploadTestEnvModeAction);
		//GM
		EnhancedInputComponent->BindAction(GKeyAction, ETriggerEvent::Started, this, &AAwsaPlayerPawn::OnGKeyPressed);
		EnhancedInputComponent->BindAction(GKeyAction, ETriggerEvent::Completed, this,
		                                   &AAwsaPlayerPawn::OnGKeyReleased);

		EnhancedInputComponent->BindAction(MKeyAction, ETriggerEvent::Started, this, &AAwsaPlayerPawn::OnMKeyPressed);
		EnhancedInputComponent->BindAction(MKeyAction, ETriggerEvent::Completed, this,
		                                   &AAwsaPlayerPawn::OnMKeyReleased);
	}
	else
	{
		UAutoParse::PrintLog_GameThread(FString::Printf(TEXT(
			"'%s' Failed to find an Enhanced Input component! "), *GetNameSafe(this)));
	}
}

AAwsaPlayerPawn* AAwsaPlayerPawn::GetAwsaPlayerPawn()
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		return Cast<AAwsaPlayerPawn>(World->GetFirstPlayerController()->GetPawn());
	}
	return nullptr;
}

void AAwsaPlayerPawn::SwitchViewMode(EViewMode TargetViewMode)
{
	CurrViewMode = TargetViewMode;
	if (CurrViewMode == EViewMode::FirstPerson)
	{
		SavedSpringArmLength = SpringArm->TargetArmLength;
		SavedControlRotation = GetControlRotation();
		SavedPawnLocation = GetActorLocation();

		SpringArm->bEnableCameraLag = false;
		SpringArm->bEnableCameraRotationLag = false;

		FVector StartCameraLocation = Camera->GetComponentLocation();
		FRotator StartCameraRotation = Camera->GetComponentRotation();
		SetActorLocationAndRotation(StartCameraLocation, StartCameraRotation);

		MovementComponent->SetPlaneConstraintEnabled(false);
		Camera->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Camera->SetRelativeLocation(FirstPersonCameraOffset);
		Camera->bUsePawnControlRotation = true;
		if (FirstPersonCollisionSphere)
		{
			SetActorEnableCollision(true);
			FirstPersonCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		SwitchMappingContext(EControlType::Fpp);
	}
	else // ThirdPerson
	{
		SpringArm->TargetArmLength = 0.f;
		MovementComponent->SetPlaneConstraintEnabled(true);

		Camera->bUsePawnControlRotation = false;
		Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::SnapToTargetIncludingScale);
		Camera->SetRelativeLocation(FVector::ZeroVector);

		SpringArm->bEnableCameraLag = true;
		SpringArm->bEnableCameraRotationLag = true;
		MoveCameraToTarget(SavedPawnLocation, SavedControlRotation, SavedSpringArmLength, 0.f);

		if (FirstPersonCollisionSphere)
		{
			SetActorEnableCollision(false);
			FirstPersonCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			FirstPersonCollisionSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		SwitchMappingContext(EControlType::Tpp);
	}
}

void AAwsaPlayerPawn::CheckMouseHover()
{
	FHitResult HitResult;
	AActor* NewHoveredActor = nullptr;

	if (GetActorUnderCursor(HitResult))
	{
		NewHoveredActor = HitResult.GetActor();
	}

	if (NewHoveredActor != CurrentHoveredActor)
	{
		//Trigger the leave event
		if (CurrentHoveredActor)
		{
			NotifyMouseLeave(CurrentHoveredActor);
			// UAutoParse::PrintLog_GameThread(
			// 	FString::Printf(TEXT("Mouse Leave: %s"), *GetNameSafe(CurrentHoveredActor)));
		}


		LastHoveredActor = CurrentHoveredActor;
		CurrentHoveredActor = NewHoveredActor;

		//Trigger the Hover event
		if (CurrentHoveredActor)
		{
			NotifyMouseOverlay(CurrentHoveredActor);
			// UAutoParse::PrintLog_GameThread(
			// 	FString::Printf(TEXT("Mouse Hover: %s"), *GetNameSafe(CurrentHoveredActor)));
		}
	}
}

void AAwsaPlayerPawn::CheckMouseHover_Component()
{
	FHitResult HitResult;
	UMeshComponent* NewHoveredComponent = nullptr;

	if (GetActorUnderCursor(HitResult))
	{
		NewHoveredComponent = Cast<UMeshComponent>(HitResult.GetComponent());
	}

	if (NewHoveredComponent != CurrentHoveredComponent)
	{
		if (CurrentHoveredComponent)
		{
			NotifyMouseLeave_Component(CurrentHoveredComponent);
		}


		LastHoveredComponent = CurrentHoveredComponent;
		CurrentHoveredComponent = NewHoveredComponent;

		if (CurrentHoveredComponent)
		{
			NotifyMouseOverlay_Component(CurrentHoveredComponent);
		}
	}
}

bool AAwsaPlayerPawn::GetActorUnderCursor(FHitResult& OutHitResult) const
{
	if (AAwsaPlayerController* PC = Cast<AAwsaPlayerController>(GetController()))
	{
		TArray<TEnumAsByte<EObjectTypeQuery>> objTypes;
		objTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));
		objTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel2));

		return PC->GetHitResultUnderCursorForObjects(objTypes, true, OutHitResult);
	}
	return false;
}


void AAwsaPlayerPawn::OnLeftMouseClicked(const FInputActionValue& Value)
{
	if (bIsPlacePreviewActive)
	{
		PlacePreviewComponent->SetDetectionActive(false);
		PlacePreviewComponent->OnLeftMouseClicked();
	}

	if (bComboActive)
	{
		RecordMousePosition();
		return;
	}

	FHitResult HitResult;

	if (GetActorUnderCursor(HitResult))
	{
		AActor* ClickedActor = HitResult.GetActor();
		NotifyMouseClick(ClickedActor, true);
		UMeshComponent* ClickedComponent = Cast<UMeshComponent>(HitResult.GetComponent());
		if (ClickedComponent)
		{
			NotifyMouseClick_Component(ClickedComponent);
		}
	}
}

void AAwsaPlayerPawn::OnLeftMouseClicked_Component(const FInputActionValue& Value)
{
	if (bIsPlacePreviewActive)
	{
		PlacePreviewComponent->SetDetectionActive(false);
		PlacePreviewComponent->OnLeftMouseClicked();
	}

	if (bComboActive)
	{
		RecordMousePosition();
		return;
	}

	FHitResult HitResult;
	if (GetActorUnderCursor(HitResult))
	{
		UMeshComponent* NewHoveredComponent = nullptr;
		NewHoveredComponent = Cast<UMeshComponent>(HitResult.GetComponent());
		if (NewHoveredComponent)
		{
			NotifyMouseClick_Component(NewHoveredComponent);
		}
	}
}

void AAwsaPlayerPawn::OnRightMouseClicked(const FInputActionValue& Value)
{
	FHitResult HitResult;
	if (GetActorUnderCursor(HitResult))
	{
		AActor* ClickedActor = HitResult.GetActor();
		NotifyMouseClick(ClickedActor, false);
	}
}


void AAwsaPlayerPawn::NotifyMouseOverlay(AActor* Actor)
{
	if (Actor && Actor->GetClass()->ImplementsInterface(UZHJZ_Interface::StaticClass()))
	{
		IZHJZ_Interface::Execute_MouseOverlay(Actor);
	}
}

void AAwsaPlayerPawn::NotifyMouseLeave(AActor* Actor)
{
	if (Actor && Actor->GetClass()->ImplementsInterface(UZHJZ_Interface::StaticClass()))
	{
		IZHJZ_Interface::Execute_MouseLeave(Actor);
	}
}

void AAwsaPlayerPawn::NotifyMouseClick(AActor* Actor, bool bIsLeftClick)
{
	if (Actor && Actor->GetClass()->ImplementsInterface(UZHJZ_Interface::StaticClass()))
	{
		IZHJZ_Interface::Execute_MouseClick(Actor, bIsLeftClick);
	}
}

void AAwsaPlayerPawn::NotifyMouseOverlay_Component(UMeshComponent* Component)
{
	if (Component->GetOwner()->GetClass()->ImplementsInterface(UZHJZ_Interface::StaticClass()))
	{
		IZHJZ_Interface::Execute_MouseOverlay_Component(Component->GetOwner(), Component);
	}
}

void AAwsaPlayerPawn::NotifyMouseLeave_Component(UMeshComponent* Component)
{
	if (Component->GetOwner()->GetClass()->ImplementsInterface(UZHJZ_Interface::StaticClass()))
	{
		IZHJZ_Interface::Execute_MouseLeave_Component(Component->GetOwner(), Component);
	}
}

void AAwsaPlayerPawn::NotifyMouseClick_Component(UMeshComponent* Component)
{
	if (Component->GetOwner()->GetClass()->ImplementsInterface(UZHJZ_Interface::StaticClass()))
	{
		IZHJZ_Interface::Execute_MouseClick_Component(Component->GetOwner(), Component);
	}
}

void AAwsaPlayerPawn::OnPlacePreviewAction(const FInputActionValue& Value)
{
	UAutoParse::PrintLog_GameThread(TEXT("Place Preview Action Triggered"));
	UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!WebUI_Subsystem)
	{
		return;
	}
	FJsonLibraryValue WebValue = FString(TEXT("false"));
	WebUI_Subsystem->OnCallWebFunc.Broadcast(TEXT("setBothSides"), WebValue);

	UEquipActorSubsystem* EquipActorSubsystem = GetGameInstance()->GetSubsystem<UEquipActorSubsystem>();
	if (!EquipActorSubsystem)
	{
		return;
	}
	EquipActorSubsystem->OnPlacePreviewMode.Broadcast(true);

	if (PlacePreviewComponent && !bIsPlacePreviewActive)
	{
		bIsPlacePreviewActive = true;
		PlacePreviewComponent->ShowMarker(true);
		PlacePreviewComponent->SetDetectionActive(true);
	}
}

void AAwsaPlayerPawn::ExitPlacePreview()
{
	UAwsaWebUI_Subsystem* WebUI_Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!WebUI_Subsystem)
	{
		return;
	}
	FJsonLibraryValue WebValue = FString(TEXT("true"));
	WebUI_Subsystem->OnCallWebFunc.Broadcast(TEXT("setBothSides"), WebValue);

	if (PlacePreviewComponent)
	{
		bIsPlacePreviewActive = false;
		PlacePreviewComponent->ShowMarker(false);
		PlacePreviewComponent->SetDetectionActive(false);
		PlacePreviewComponent->ShowWidget(false);
	}
}

void AAwsaPlayerPawn::OnUploadTestEnvModeAction(const FInputActionValue& Value)
{
	UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!WebUI_Subsystem)
	{
		return;
	}

	if (WebUI_Subsystem->OnTestEnvHotLoad.IsBound())
	{
		WebUI_Subsystem->OnTestEnvHotLoad.Broadcast();
	}
}

void AAwsaPlayerPawn::OnGKeyPressed(const FInputActionValue& Value)
{
	bGKeyPressed = true;
	UE_LOG(LogTemp, Log, TEXT("G Key Pressed"));
}

void AAwsaPlayerPawn::OnGKeyReleased(const FInputActionValue& Value)
{
	bGKeyPressed = false;
	UE_LOG(LogTemp, Log, TEXT("G Key Released"));
}

void AAwsaPlayerPawn::OnMKeyPressed(const FInputActionValue& Value)
{
	bMKeyPressed = true;
	UE_LOG(LogTemp, Log, TEXT("M Key Pressed"));
}

void AAwsaPlayerPawn::OnMKeyReleased(const FInputActionValue& Value)
{
	bMKeyPressed = false;
	UE_LOG(LogTemp, Log, TEXT("M Key Released"));
}

void AAwsaPlayerPawn::RecordMousePosition()
{
	if (AAwsaPlayerController* PC = Cast<AAwsaPlayerController>(GetController()))
	{
		FVector WorldPosition;
		if (PC->GetMousePositionInWorld(WorldPosition))
		{
			UE_LOG(LogTemp, Display, TEXT("Recorded position: %s"), *WorldPosition.ToString());

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					5.0f,
					FColor::Yellow,
					FString::Printf(TEXT("Position recorded: X=%.2f, Y=%.2f, Z=%.2f"),
					                WorldPosition.X, WorldPosition.Y, WorldPosition.Z)
				);
			}
			ShowPositionInUI(WorldPosition);
			DrawDebugSphere(GetWorld(), WorldPosition, 50.0f, 12, FColor::Green, false, 5.0f);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to get mouse position in world"));
		}
	}
}

void AAwsaPlayerPawn::MoveCameraToTarget(FVector TargetLocation, FRotator TargetRotation, float TargetArmLengthValue,
                                         float Duration)
{
	if (Duration <= 0.0f)
	{
		SetActorLocation(TargetLocation);
		SpringArm->TargetArmLength = TargetArmLengthValue;
		TargetArmLength = TargetArmLengthValue;
		if (Controller)
		{
			Controller->SetControlRotation(TargetRotation);
		}
		return;
	}

	TargetCameraLocation = TargetLocation;
	TargetCameraArmLength = FMath::Clamp(TargetArmLengthValue, MinTargetArmLength, MaxTargetArmLength);
	TargetCameraRotation = TargetRotation;
	MoveToTargetDuration = Duration;
	CurrentMoveTime = 0.0f;

	StartLocation = GetActorLocation();
	StartArmLength = SpringArm->TargetArmLength;
	if (Controller)
	{
		StartRotation = Controller->GetControlRotation();
	}

	bIsMovingToTarget = true;
}

void AAwsaPlayerPawn::MovePawnToTarget(FVector TargetLocation, FRotator TargetRotation, float Duration)
{
	if (Duration <= 0.0f)
	{
		SetActorLocation(TargetLocation);
		if (Controller)
		{
			Controller->SetControlRotation(TargetRotation);
		}
		return;
	}

	TargetCameraLocation = TargetLocation;
	TargetCameraRotation = TargetRotation;
	MoveToTargetDuration = Duration;
	CurrentMoveTime = 0.0f;

	StartLocation = GetActorLocation();
	if (Controller)
	{
		StartRotation = Controller->GetControlRotation();
	}
	bIsMovingToTarget = true;
}

void AAwsaPlayerPawn::OnPlacePreviewMode(bool bPlacePreviewMode)
{
	if (!bPlacePreviewMode)
	{
		ExitPlacePreview();
	}
}


void AAwsaPlayerPawn::ShowPositionInUI(const FVector& Position)
{
	if (PositionWidgetClass)
	{
		if (!PositionWidget)
		{
			PositionWidget = CreateWidget<UPositionDisplayUI>(GetWorld(), PositionWidgetClass);
		}
		if (PositionWidget)
		{
			PositionWidget->AddToViewport();
			PositionWidget->SetPositionText(Position);
			PositionWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void AAwsaPlayerPawn::SetCameraMoveSpeed(float TargetMoveSpeed)
{
	MovementComponent->MaxSpeed = TargetMoveSpeed;
}

void AAwsaPlayerPawn::SwitchMappingContext(EControlType TargetType)
{
	if (CurrControlType == TargetType)
	{
		return;
	}
	if (APlayerController* PlayerController = Cast<AAwsaPlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (CurrMappingContext)
			{
				Subsystem->RemoveMappingContext(CurrMappingContext);
				CurrMappingContext = nullptr;
			}

			if (TargetType == EControlType::Default && DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 1);
				CurrMappingContext = DefaultMappingContext;
				CurrControlType = TargetType;
				return;
			}
			if (TargetType == EControlType::Fpp && FppMappingContext)
			{
				Subsystem->AddMappingContext(FppMappingContext, 1);
				CurrMappingContext = FppMappingContext;
				CurrControlType = TargetType;
				return;
			}
			if (TargetType == EControlType::Tpp && TppMappingContext)
			{
				Subsystem->AddMappingContext(TppMappingContext, 1);
				CurrMappingContext = TppMappingContext;
				CurrControlType = TargetType;
				return;
			}
			if (TargetType == EControlType::Observer && ObserverMappingContext)
			{
				Subsystem->AddMappingContext(ObserverMappingContext, 1);
				CurrMappingContext = ObserverMappingContext;
				CurrControlType = TargetType;
				return;
			}
			if (TargetType == EControlType::Disable && DisableMappingContext)
			{
				Subsystem->AddMappingContext(DisableMappingContext, 1);
				CurrMappingContext = DisableMappingContext;
				CurrControlType = TargetType;
				return;
			}
		}
	}
}

void AAwsaPlayerPawn::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	UpdateMovementSpeedByCameraHeight();

	if (Controller != nullptr && !MovementVector.IsNearlyZero())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		FVector DeltaLocation = (-MovementVector.Y * ForwardDirection + -MovementVector.X * RightDirection) *
			CurrentMoveSpeed;
		AddActorWorldOffset(DeltaLocation, true);
	}
}

void AAwsaPlayerPawn::Rotator(const FInputActionValue& Value)
{
	FVector2D RotatorAxisVector2D = Value.Get<FVector2D>();
	FRotator CurrentRot = Controller->GetControlRotation();
	float NewPitch = FMath::UnwindDegrees(CurrentRot.Pitch) + RotatorAxisVector2D.Y;
	NewPitch = FMath::Clamp(NewPitch, MinPitch, MaxPitch);
	float NewYaw = CurrentRot.Yaw + RotatorAxisVector2D.X;
	FRotator NewRotator = FRotator(NewPitch, NewYaw, 0.f);
	Controller->SetControlRotation(NewRotator);
}

void AAwsaPlayerPawn::Scaling(const FInputActionValue& Value)
{
	float Scale = Value.Get<float>();

	ScaleSpeed = FMath::GetMappedRangeValueClamped(FVector2D(MinTargetArmLength, MaxTargetArmLength),
	                                               FVector2D(MinScaleSpeed, MaxScaleSpeed), TargetArmLength);
	//UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("%f"), TargetArmLength));
	TargetArmLength = FMath::Clamp(TargetArmLength += -Scale * ScaleSpeed, MinTargetArmLength,
	                               MaxTargetArmLength);

	SpringArm->TargetArmLength = TargetArmLength;
}

void AAwsaPlayerPawn::FppMove(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}
	FRotator ControlRot = Controller->GetControlRotation();
	FVector Forward = ControlRot.Vector();
	// FVector Right = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);
	FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

	AddMovementInput(Forward, Input.Y);
	AddMovementInput(Right, Input.X);
}

void AAwsaPlayerPawn::FppLook(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (Controller)
	{
		FRotator CurrentRot = Controller->GetControlRotation();
		float Sensitivity = FppLookSensitivity;
		float NewPitch = FMath::Clamp(CurrentRot.Pitch + Input.Y * Sensitivity, MinPitch, -MinPitch);
		float NewYaw = CurrentRot.Yaw + Input.X * Sensitivity;
		FRotator NewRotator = FRotator(NewPitch, NewYaw, 0.f);
		Controller->SetControlRotation(NewRotator);
	}
}

void AAwsaPlayerPawn::UpdateMovementSpeedByCameraHeight()
{
	if (!Camera || !MovementComponent)
	{
		return;
	}

	float CameraZ = Camera->GetComponentLocation().Z;

	float MinCameraZ = -2200.f;
	float MaxCameraZ = 150000.f;

	float Speed = FMath::GetMappedRangeValueClamped(
		FVector2D(MinCameraZ, MaxCameraZ),
		FVector2D(MinMoveSpeed, MaxMoveSpeed),
		CameraZ
	);
	CurrentMoveSpeed = Speed;
}

void AAwsaPlayerPawn::InitCameraMoving()
{
	MoveCameraToTarget(InitCameraEndLocation,InitCameraEndRotation,InitCameraEndArmLength,InitCameraEndDuration);
}

void AAwsaPlayerPawn::OnExplosionAnimation(const FString& OwnerName, bool bIsMultiple)
{
	FExplosionTransform Instance = CameraTransformMap.FindRef(OwnerName);
	if (bIsMultiple)
	{
		MoveCameraToTarget(Instance.Location,Instance.Rotation, Instance.CameraArmLength,1.f);
	}
}

void AAwsaPlayerPawn::OnModuleSwitched(const FString& NewModuleName, const FString& InExtraParam)
{
	// 超前地质预报
	if (NewModuleName.Equals(TEXT("超前地质预报"), ESearchCase::CaseSensitive))
	{
		FExplosionTransform Instance = CameraTransformMap.FindRef(FString::Printf(TEXT("超前地质预报_%s"),*InExtraParam));
		MoveCameraToTarget(Instance.Location,Instance.Rotation, Instance.CameraArmLength,1.f);
	}
	if (NewModuleName.Equals(TEXT("隧道超欠挖"), ESearchCase::CaseSensitive))
	{
		FExplosionTransform Instance = CameraTransformMap.FindRef(FString::Printf(TEXT("超前地质预报_%s"),*InExtraParam));
		MoveCameraToTarget(Instance.Location,Instance.Rotation, Instance.CameraArmLength,1.f);
	}
	if (NewModuleName.Equals(TEXT("围岩变形监测"), ESearchCase::CaseSensitive))
	{
		FExplosionTransform Instance = CameraTransformMap.FindRef(FString::Printf(TEXT("超前地质预报_%s"),*InExtraParam));
		MoveCameraToTarget(Instance.Location,Instance.Rotation, Instance.CameraArmLength,1.f);
	}
}

void AAwsaPlayerPawn::OnSectionCuttingSelected(const FString& TargetTunnelName)
{
	FExplosionTransform Instance = CameraTransformMap.FindRef(FString::Printf(TEXT("剖切_%s"),*TargetTunnelName));
	MoveCameraToTarget(Instance.Location,Instance.Rotation, Instance.CameraArmLength,1.f);
}
