#include "PlacePreviewMarker.h"
#include "PlacePreviewPoi.h"

#include "Components/WidgetComponent.h"


APlacePreviewMarker::APlacePreviewMarker()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootComp;

	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComp->SetupAttachment(RootComp);
	WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComp->SetVisibility(false);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComp->SetupAttachment(RootComponent);
}

void APlacePreviewMarker::BeginPlay()
{
	Super::BeginPlay();

	UStaticMesh* ArrowMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/ZHJZPlugin/SM_DemoArrow.SM_DemoArrow"));
	if (MeshComp && ArrowMesh)
	{
		MeshComp->SetStaticMesh(ArrowMesh);
	}

	TSubclassOf<UUserWidget> PoiWidgetClass = LoadClass<UUserWidget>(
		nullptr, TEXT("/ZHJZPlugin/W_PreviewPoi.W_PreviewPoi_C"));

	if (PoiWidgetClass)
	{
		UE_LOG(LogTemp,Warning,TEXT("APlacePreviewMarker::PoiWidgetClass LOAD SUCCESS"));
		WidgetComp->SetWidgetClass(PoiWidgetClass);
		WidgetComp->InitWidget();
		PlacePreviewPoiWidget = Cast<UPlacePreviewPoi>(WidgetComp->GetWidget());
		WidgetComp->SetDrawSize(FVector2D(440.0, 260.0));
	}
	else
	{
		PlacePreviewPoiWidget = nullptr;
	}


	BaseLocation = GetActorLocation();
}

void APlacePreviewMarker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsBouncing)
	{
		RunningTime += DeltaTime;
		float DeltaZ = Amplitude * FMath::Sin(RunningTime * Frequency * 2 * PI);
		FVector NewLocation = BaseLocation + FVector(0, 0, DeltaZ);
		SetActorLocation(NewLocation, false);
	}
}

void APlacePreviewMarker::InitMarkerMesh(UStaticMesh* TargetMesh)
{
	if (TargetMesh)
	{
		MeshComp->SetStaticMesh(TargetMesh);
	}
}

void APlacePreviewMarker::SetBaseLocation(const FVector& NewLocation)
{
	BaseLocation = NewLocation;
}

void APlacePreviewMarker::StartBouncing()
{
	bIsBouncing = true;
	SetActorHiddenInGame(false);
}

void APlacePreviewMarker::StopBouncing()
{
	bIsBouncing = false;
	SetActorHiddenInGame(true);
}

void APlacePreviewMarker::ShowPoiWidget(bool bShow)
{
	WidgetComp->SetVisibility(bShow);
}

void APlacePreviewMarker::OnRaycastHit(FVector HitLocation, AActor* HitActor, UPrimitiveComponent* HitComponent)
{
	SetActorLocation(HitLocation);
}
