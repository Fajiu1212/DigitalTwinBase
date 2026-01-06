#include "Actor/StaticBasedTunnelProgress.h"

#include "AwsaWebUI_Subsystem.h"


AStaticBasedTunnelProgress::AStaticBasedTunnelProgress()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	StaticTunnelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticTunnelMeshComponent"));
	StaticTunnelMeshComponent->SetupAttachment(RootComponent);
	StaticTunnelMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	StaticTunnelMeshComponent->SetMaterial(0,SourceMaterial);
}

#if WITH_EDITOR
void AStaticBasedTunnelProgress::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(
			AStaticBasedTunnelProgress, SourceStaticMesh))
	{
		if (SourceStaticMesh && StaticTunnelMeshComponent)
		{
			StaticTunnelMeshComponent->SetStaticMesh(SourceStaticMesh);
		}
		if (GEditor)
		{
			GEditor->NoteSelectionChange();
			GEditor->RedrawAllViewports();
		}
	}
	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(
			AStaticBasedTunnelProgress, SourceMaterial))
	{
		if (SourceMaterial && StaticTunnelMeshComponent)
		{
			StaticTunnelMeshComponent->SetMaterial(0,SourceMaterial);
		}
		if (GEditor)
		{
			GEditor->NoteSelectionChange();
			GEditor->RedrawAllViewports();
		}
	}
}
#endif

void AStaticBasedTunnelProgress::BeginPlay()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		Subsystem->OnProgressLoadSuccess.AddDynamic(this, &AStaticBasedTunnelProgress::OnLoadSuccess);
		Subsystem->ModuleSwitched.AddDynamic(this, &AStaticBasedTunnelProgress::OnModuleSwitched);
		Subsystem->OnExplosionAnimation.AddDynamic(this, &AStaticBasedTunnelProgress::OnExplosionAnimation);
	}
	BasicLoc = GetActorLocation();
	Super::BeginPlay();
}

void AStaticBasedTunnelProgress::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AStaticBasedTunnelProgress::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void AStaticBasedTunnelProgress::OnLoadSuccess(const FString& TargetModuleName)
{
	if (TargetModuleName.Equals(ModuleName, ESearchCase::CaseSensitive))
	{
		SetActorLocation(BasicLoc);
		SetActorHiddenInGame(false);
	}
}

void AStaticBasedTunnelProgress::OnModuleSwitched(const FString& NewModuleName, const FString& InExtraParam)
{
	if (NewModuleName.Equals(TEXT("进度管理"), ESearchCase::CaseSensitive))
	{
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
	}
	else
	{
		if (bExplosion)
		{
			SetActorLocation(BasicLoc);
		}
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void AStaticBasedTunnelProgress::OnExplosionAnimation(const FString& OwnerName, bool bIsMultiple)
{
	if (OwnerName == TEXT("Reset"))
	{
		if (GetWorldTimerManager().IsTimerActive(MoveTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		}
		bExplosion = false;
		SetActorLocation(BasicLoc);
		SetActorHiddenInGame(false);
		return;
	}
	if (OwnerName != TunnelName)
	{
		if (GetWorldTimerManager().IsTimerActive(MoveTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		}
		SetActorHiddenInGame(true);
		if (bExplosion)
		{
			SetActorLocation(BasicLoc);
		}
		bExplosion = false;
		return;
	}
	SetActorHiddenInGame(false);
	MoveActorZSmoothly(false);
}

void AStaticBasedTunnelProgress::MoveActorZSmoothly(bool bMoveUp)
{
	if (GetWorldTimerManager().IsTimerActive(MoveTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	}
	bExplosion = true;
	bIsMovingUp = bMoveUp;
	ElapsedTime = 0.0f;
	TargetZOffset = bIsMovingUp? MoveDistance : -MoveDistance;

	GetWorldTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&AStaticBasedTunnelProgress::UpdateMove,
		0.016f,
		true
	);
}

void AStaticBasedTunnelProgress::UpdateMove()
{
	ElapsedTime += 0.016f;

	float Progress = FMath::Clamp(ElapsedTime / MoveDuration, 0.0f, 1.0f);

	float CurrentZOffset = FMath::Lerp(0, TargetZOffset, Progress);
	SetActorLocation(BasicLoc+FVector(0.0f, 0.0f, CurrentZOffset));

	if (Progress >= 1.0f)
	{
		SetActorLocation(BasicLoc+FVector(0.0f, 0.0f, TargetZOffset));
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	}
}
