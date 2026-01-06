#include "Actor/BeamActorBase.h"

#include "AwsaWebUI_Subsystem.h"
#include "JsonLibraryValue.h"
#include "Components/WidgetComponent.h"
#include "Engine/StreamableManager.h"
#include "Gameplay/AwsaPlayerPawn.h"
#include "Kismet/GameplayStatics.h"
#include "UI/BeamWidgetBase.h"

ABeamActorBase::ABeamActorBase()
{
	PrimaryActorTick.bCanEverTick = true;
	// Root comp
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	// Mesh comp
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	// Widget comp
	PoiWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("POIWidgetComponent"));
	PoiWidgetComponent->SetupAttachment(RootComponent);
	PoiWidgetComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	PoiWidgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	PoiWidgetComponent->SetRelativeLocation(FVector(0, 0, 300));
	PoiWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	PoiWidgetComponent->SetDrawSize(WidgetDrawSize);
	PoiWidgetComponent->SetRelativeLocation(WidgetOffset);
	PoiWidgetComponent->SetPivot(FVector2D(0.5, 1.f));
}

void ABeamActorBase::BeginPlay()
{
	Super::BeginPlay();
	if (PoiWidgetComponent && PoiWidgetClass)
	{
		PoiWidgetComponent->SetWidgetClass(PoiWidgetClass);
	}
	ToggleVisibility(true);
	UpdateMesh();
}

#if WITH_EDITOR
void ABeamActorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateMesh();
}

void ABeamActorBase::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABeamActorBase, BeamMesh))
	{
		UpdateMesh();
	}
}
#endif

void ABeamActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABeamActorBase::MouseOverlay_Implementation()
{
	IZHJZ_Interface::MouseOverlay_Implementation();
	if (BeamState == EBeamActorState::HighlightShowMenu ||
		BeamState == EBeamActorState::ShowMenu)
	{
		return;
	}

	if (UBeamGeneratorSubsystem* BeamManagerSubsystem = GetGameInstance()->GetSubsystem<UBeamGeneratorSubsystem>())
	{
		if (BeamManagerSubsystem->bAllowPoi)
		{
			OnBeamStateChanged(EBeamActorState::HighlightShowPoi);
		}
	}
}

void ABeamActorBase::MouseLeave_Implementation()
{
	IZHJZ_Interface::MouseLeave_Implementation();
	if (BeamState == EBeamActorState::HighlightShowMenu ||
		BeamState == EBeamActorState::ShowMenu)
	{
		return;
	}
	OnBeamStateChanged(EBeamActorState::Default);
}

void ABeamActorBase::MouseClick_Implementation(bool isBool)
{
	IZHJZ_Interface::MouseClick_Implementation(isBool);

	if (UBeamGeneratorSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UBeamGeneratorSubsystem>())
	{
		Subsystem->OnAllBeamReset.Broadcast();
	}

	OnBeamActorClicked();
}

void ABeamActorBase::OnBeamStateChanged(EBeamActorState TargetState)
{
	switch (TargetState)
	{
	case EBeamActorState::Default:
		{
			HighlightEffect(false);
			TogglePoi(false);
			BeamState = TargetState;
			break;
		}
	case EBeamActorState::Highlight:
		{
			HighlightEffect(true);
			TogglePoi(false);
			BeamState = TargetState;
			break;
		}
	case EBeamActorState::HighlightShowPoi:
		{
			HighlightEffect(true);
			TogglePoi(true);
			BeamState = TargetState;
			break;
		}
	case EBeamActorState::ShowMenu:
		{
			HighlightEffect(false);
			TogglePoi(false);
			BeamState = TargetState;
			break;
		}
	case EBeamActorState::HighlightShowMenu:
		{
			HighlightEffect(true);
			TogglePoi(false);
			BeamState = TargetState;
			break;
		}
	default: break;
	}
}

void ABeamActorBase::HighlightEffect(bool bIsHighlight)
{
	if (MeshComponent)
	{
		MeshComponent->SetRenderCustomDepth(bIsHighlight);
	}
	if (BeamType == EAiType::AiPedestal)
	{
		HightlightSkeletalMesh(bIsHighlight);
	}
}

void ABeamActorBase::TogglePoi(bool bIsPoiVisible)
{
	if (PoiWidgetComponent)
	{
		PoiWidgetComponent->SetHiddenInGame(!bIsPoiVisible);
	}
}

void ABeamActorBase::ChangeBeamState(EBeamActorState NewState)
{
	OnBeamStateChanged(NewState);
}

void ABeamActorBase::ResetBeamActor()
{
	if (BeamState == EBeamActorState::Default)
	{
		return;
	}
	OnBeamStateChanged(EBeamActorState::Default);
}

void ABeamActorBase::InitBeamActor()
{
	ToggleVisibility(true);
}

void ABeamActorBase::SetBeamStruct(const FBeamStruct& NewBeamStruct)
{
	BeamStruct = NewBeamStruct;
	if (PoiWidgetComponent)
	{
		if (UBeamWidgetBase* PoiWidget = Cast<UBeamWidgetBase>(PoiWidgetComponent->GetWidget()))
		{
			PoiWidget->SetBeamStruct(BeamStruct);
		}
	}
}

void ABeamActorBase::ToggleVisibility(bool bIsVisible)
{
	SetActorEnableCollision(bIsVisible);
	SetActorHiddenInGame(!bIsVisible);
	if (PoiWidgetComponent)
	{
		if (bIsVisible)
		{
			if (BeamState == EBeamActorState::HighlightShowPoi)
			{
				PoiWidgetComponent->SetHiddenInGame(false);
			}
		}
		else
		{
			PoiWidgetComponent->SetHiddenInGame(true);
		}
	}
}

void ABeamActorBase::UpdateMesh()
{
	if (BeamMesh.IsNull())
	{
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(nullptr);
		}
		return;
	}
	
	FSoftObjectPath MeshPath = BeamMesh.ToSoftObjectPath();
	if (!MeshPath.IsValid()) return;
	FStreamableManager StreamableManager;
	TWeakObjectPtr<ABeamActorBase> WeakThis(this);
	StreamableManager.RequestAsyncLoad(MeshPath, [WeakThis]()
	{
		if (!WeakThis.IsValid()) return;
		UStaticMesh* Mesh = WeakThis->BeamMesh.Get();
		if (Mesh && WeakThis->MeshComponent)
		{
			WeakThis->MeshComponent->SetStaticMesh(Mesh);
		}
	});
}

void ABeamActorBase::UpdateFromBeamStruct(const FBeamStruct& NewBeamStruct)
{
	BeamStruct = NewBeamStruct;
	if (PoiWidgetComponent)
	{
		if (UBeamWidgetBase* PoiWidget = Cast<UBeamWidgetBase>(PoiWidgetComponent->GetWidget()))
		{
			PoiWidget->SetBeamStruct(BeamStruct);
		}
	}
}

void ABeamActorBase::ChangeAiMode_Implementation(bool bIsAiMode)
{
}

void ABeamActorBase::CallWebCenterPopup_Implementation()
{
	// Can be overridden in Blueprint
	if (UAwsaWebUI_Subsystem* WebUISubsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		FString FuncName = TEXT("setCenterPopup");
		FJsonLibraryValue Data = {};
		WebUISubsystem->CallWebFunc(FuncName, Data);
	}
}

void ABeamActorBase::MoveToBeam_Implementation()
{
	// Can be overridden in Blueprint
	if (AAwsaPlayerPawn* PlayerPawn = Cast<AAwsaPlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		PlayerPawn->MoveCameraToTarget(GetActorLocation(), PlayerPawn->GetControlRotation(), MovementTargetArmLength,
		                               MovementDuration);
	}
}

void ABeamActorBase::OnBeamActorClicked_Implementation()
{
	// Can be overridden in Blueprint
	OnBeamStateChanged(EBeamActorState::HighlightShowMenu);
	MoveToBeam();
	CallWebCenterPopup();
}
