#include "Actor/BaseBillboardActor.h"

#include "Components/WidgetComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Gameplay/AwsaPlayerPawn.h"
#include "Kismet/GameplayStatics.h"


ABaseBillboardActor::ABaseBillboardActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	MeshComponentsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MeshComponentsRoot"));
	MeshComponentsRoot->SetupAttachment(RootComponent);

	BillboardWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	BillboardWidgetComp->SetupAttachment(MeshComponentsRoot);
	BillboardWidgetComp->SetRelativeLocation(FVector(0, 0, 590));
	BillboardWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	BillboardWidgetComp->SetDrawSize(WidgetDrawSize);
	BillboardWidgetComp->SetPivot(FVector2D(0.5, 1.f));
	BillboardWidgetComp->SetCollisionObjectType(ECC_GameTraceChannel2);
}

void ABaseBillboardActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateMeshComponents();
	SetWidget();
	SetBillboardIcon();
}

#if WITH_EDITOR
void ABaseBillboardActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() ==
		GET_MEMBER_NAME_CHECKED(ABaseBillboardActor, BillboardStaticMeshes))
	{
		UpdateMeshComponents();
		SetWidget();
		SetBillboardIcon();
	}
	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() ==
		GET_MEMBER_NAME_CHECKED(ABaseBillboardActor, BillboardWidget))
	{
		SetWidget();
	}
	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() ==
		GET_MEMBER_NAME_CHECKED(ABaseBillboardActor, BillboardIcon))
	{
		SetBillboardIcon();
	}
}
#endif

void ABaseBillboardActor::BeginPlay()
{
	Super::BeginPlay();
	GetMainPawn();
	SetBillboardIcon();
	GetWorld()->GetTimerManager().SetTimer(RotateTimerHandle, this, &ABaseBillboardActor::FaceCameraAndFitScale, 0.02f,
	                                       true, 0.1f);
}

void ABaseBillboardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseBillboardActor::MouseClick_Implementation(bool isBool)
{
	IZHJZ_Interface::MouseClick_Implementation(isBool);
	OnMouseClicked(isBool);
}

AAwsaPlayerPawn* ABaseBillboardActor::GetMainPawn()
{
	if (!PlayerPawn && GetWorld())
	{
		PlayerPawn = Cast<AAwsaPlayerPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	}
	return PlayerPawn;
}

void ABaseBillboardActor::UpdateMeshComponents()
{
	if (!MeshComponentsRoot)
	{
		return;
	}

	TArray<UStaticMeshComponent*> OldMeshComponents;
	GetComponents<UStaticMeshComponent>(OldMeshComponents);
	for (UStaticMeshComponent* Comp : OldMeshComponents)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}

	for (UStaticMesh* Mesh : BillboardStaticMeshes)
	{
		if (Mesh)
		{
			UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(
				this, UStaticMeshComponent::StaticClass(), NAME_None, RF_Transactional);
			MeshComp->SetStaticMesh(Mesh);
			MeshComp->SetupAttachment(MeshComponentsRoot);
			MeshComp->RegisterComponent();
			MeshComp->SetRelativeLocation(FVector::ZeroVector);
			MeshComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
			AddInstanceComponent(MeshComp);
		}
	}
}

void ABaseBillboardActor::SetWidget()
{
	if (BillboardWidgetComp)
	{
		BillboardWidgetComp->SetWidgetClass(BillboardWidget);
	}
}

void ABaseBillboardActor::FaceCameraAndFitScale()
{
	APlayerCameraManager* CameraManger = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!CameraManger)
	{
		return;
	}
	FVector BillboardLoc = BillboardWidgetComp->GetComponentLocation();
	FVector CameraLoc = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
	FRotator TargetRot = FRotationMatrix::MakeFromX(CameraLoc - BillboardLoc).Rotator();
	float ClampRoll = FMath::Clamp(TargetRot.Roll, 0.0, 45.0);
	float ClampPitch = FMath::Clamp(TargetRot.Pitch, 0.0, 45.0);
	BillboardWidgetComp->SetWorldRotation(FRotator(ClampPitch, TargetRot.Yaw, ClampRoll));

	float CurrArmLength = GetMainPawn()->TargetArmLength;
	if (LastArmLength == CurrArmLength)
	{
		return;
	}
	LastArmLength = CurrArmLength;
	AdjustedScale = LastArmLength / RefArmLength;
	float ClampedScale = FMath::Clamp(AdjustedScale, ClampMinScale, ClampMaxScale);
	BillboardWidgetComp->SetWorldScale3D(FVector(1, ClampedScale, ClampedScale));
}

void ABaseBillboardActor::ToggleBillboardVisibility(bool bShow)
{
	SetActorHiddenInGame(!bShow);
	if (BillboardBindAura)
	{
		BillboardBindAura->SetActorHiddenInGame(bShow);
	}
}

void ABaseBillboardActor::OnMouseClicked_Implementation(bool bIsBool)
{
	ToggleBillboardVisibility(false);
	if (GetMainPawn())
	{
		PlayerPawn->MoveCameraToTarget(GetActorLocation(), GetMainPawn()->GetControlRotation(), 8000.f, 1.2f);
	}
}
