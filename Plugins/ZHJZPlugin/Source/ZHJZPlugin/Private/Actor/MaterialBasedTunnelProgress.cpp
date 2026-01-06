

#include "Actor/MaterialBasedTunnelProgress.h"

#include "AwsaWebUI_Subsystem.h"
#include "JsonLibraryValue.h"
#include "Components/SplineComponent.h"


AMaterialBasedTunnelProgress::AMaterialBasedTunnelProgress()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	PoiSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("PoiSplineComponent"));
	PoiSplineComponent->SetupAttachment(RootComponent);
}

void AMaterialBasedTunnelProgress::BeginPlay()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		Subsystem->OnSlabLoadSuccess.AddDynamic(this, &AMaterialBasedTunnelProgress::OnLoadSuccess);
		Subsystem->ModuleSwitched.AddDynamic(this,&AMaterialBasedTunnelProgress::OnModuleSwitched);
		Subsystem->OnExplosionAnimation.AddDynamic(this,&AMaterialBasedTunnelProgress::OnExplosionAnimation);
		Subsystem->OnSectionCuttingSelected.AddDynamic(this,&AMaterialBasedTunnelProgress::OnSectionCuttingSelected);
	}
	BasicLoc = GetActorLocation();
	Super::BeginPlay();
}

void AMaterialBasedTunnelProgress::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMaterialBasedTunnelProgress::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
#if WITH_EDITOR
	TArray<UActorComponent*> ComponentsToRemove;
	for (UActorComponent* Comp : GetComponents())
	{
		if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Comp))
		{
			if (SMC != RootComponent)
			{
				ComponentsToRemove.Add(SMC);
			}
		}
	}
	for (UActorComponent* Comp : ComponentsToRemove)
	{
		if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Comp))
		{
			SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SMC->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
			RemoveInstanceComponent(SMC);
			SMC->DestroyComponent();
		}
	}
	// UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this,UStaticMeshComponent::StaticClass(), NAME_None,RF_Transactional);
	// if (NewComp && SourceStaticMesh)
	// {
	// 	NewComp->SetStaticMesh(SourceStaticMesh);
	// 	NewComp->SetupAttachment(RootComponent);
	// 	NewComp->RegisterComponent();
	// 	NewComp->SetRelativeLocation(FVector::ZeroVector);
	// 	NewComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	// 	AddInstanceComponent(NewComp);
	// 	Modify();
	// }
#endif
}
#if WITH_EDITOR
void AMaterialBasedTunnelProgress::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void AMaterialBasedTunnelProgress::OnLoadSuccess(int32 InSlabIndex, const FString& TargetModuleName)
{
	if (InSlabIndex != SLabIndex) return;

	if (TargetModuleName.Equals(ModuleName, ESearchCase::CaseSensitive))
	{
		if (!CreatedStaticeMeshComponents.IsEmpty())
		{
			for (int32 Index = CreatedStaticeMeshComponents.Num() - 1; Index >= 0; --Index)
			{
				UStaticMeshComponent* Comp = CreatedStaticeMeshComponents[Index];
				if (IsValid(Comp))
				{
					Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					Comp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
					RemoveInstanceComponent(Comp);
					Comp->DestroyComponent();
				}
			}
			CreatedStaticeMeshComponents.Reset();
		}
	
		if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
		{
			TArray<FSingleTunnelFloorInfo> CompletedFloors;
			float TotalDistance = 0.f;
			if (Subsystem->FindCompletedFloorsByTunnelName(TunnelName,CompletedFloors,TotalDistance))
			{
				for (auto FloorInfo : CompletedFloors)
				{
					float StartPercent = FloorInfo.EndMileage/TotalDistance;
					StartPercent = FMath::Clamp(StartPercent, 0.011f,0.999f);
					float EndPercent = FloorInfo.BeginMileage/TotalDistance;
					EndPercent = FMath::Clamp(EndPercent, 0.011f,0.999f);
				
					UMaterialInstanceDynamic* DynamicMtl = UMaterialInstanceDynamic::Create(Material_Progress,this);
					if (bFlip)
					{
						DynamicMtl->SetScalarParameterValue(TEXT("前消散"),1-EndPercent);
						DynamicMtl->SetScalarParameterValue(TEXT("后消散"),1-StartPercent);
					}
					else
					{
						DynamicMtl->SetScalarParameterValue(TEXT("前消散"),StartPercent);
						DynamicMtl->SetScalarParameterValue(TEXT("后消散"),EndPercent);
					}
					
					UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("%f,%f"),StartPercent,EndPercent));
				
					UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this,UStaticMeshComponent::StaticClass(), NAME_None,RF_Transactional);
					if (NewComp)
					{
						NewComp->SetStaticMesh(SourceStaticMesh);
						NewComp->SetMaterial(0,DynamicMtl);
						NewComp->SetupAttachment(RootComponent);
						NewComp->RegisterComponent();
						NewComp->SetRelativeLocation(FVector::ZeroVector);
						NewComp->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
						AddInstanceComponent(NewComp);
						CreatedStaticeMeshComponents.Emplace(NewComp);
					}
				}
			}
		}
		if (TunnelName == TEXT("2#导流洞"))
		{
			// TO DO : Hard core
			SetActorHiddenInGame(false);
		}
	}
}

void AMaterialBasedTunnelProgress::OnModuleSwitched(const FString& NewModuleName, const FString& InExtraParam)
{
	if ( NewModuleName == TEXT("进度管理") && InExtraParam.Equals(ModuleName, ESearchCase::CaseSensitive))
	{
		SetActorHiddenInGame(false);
	}
	else
	{
		if (bExplosion)
		{
			SetActorLocation(BasicLoc);
		}
		SetActorHiddenInGame(true);
	}
}

void AMaterialBasedTunnelProgress::OnExplosionAnimation(const FString& OwnerName, bool bIsMultiple)
{
	if (OwnerName == TEXT("Reset") && ModuleName.Equals(TEXT("进度管理"), ESearchCase::CaseSensitive))
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
	if (OwnerName != TunnelName && ModuleName.Equals(TEXT("进度管理"), ESearchCase::CaseSensitive))
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
	if (ModuleName.Equals(TEXT("进度管理"), ESearchCase::CaseSensitive))
	{
		SetActorHiddenInGame(false);
		MoveActorZSmoothly(false);
	}
}

void AMaterialBasedTunnelProgress::MoveActorZSmoothly(bool bMoveUp)
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
		&AMaterialBasedTunnelProgress::UpdateMove,
		0.016f,
		true
	);
}

void AMaterialBasedTunnelProgress::UpdateMove()
{
	ElapsedTime += 0.016f;

	float Progress = FMath::Clamp(ElapsedTime / MoveDuration, 0.0f, 1.0f);

	float CurrentZOffset = FMath::Lerp(0, TargetZOffset, Progress);
	SetActorLocation(BasicLoc+FVector(0.0f, 0.0f, CurrentZOffset));

	if (Progress >= 1.0f)
	{
		SetActorLocation(BasicLoc+FVector(0.0f, 0.0f, TargetZOffset));
		// 由AwsaWebUI_Subsystem处理
		// if (UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
		// {
		// 	WebUI_Subsystem->CallWebFunc(TEXT("OnLabelClickComplete"), FJsonLibraryValue());
		// }
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	}
}

void AMaterialBasedTunnelProgress::OnSectionCuttingSelected(const FString& TargetTunnelName)
{
	if (ModuleName.Equals(TEXT("剖面")))
	{
		if (TargetTunnelName.Equals(TunnelName, ESearchCase::CaseSensitive))
		{
			SetActorLocation(BasicLoc);
			SetActorHiddenInGame(false);
			UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("OwnerTunnelName:%s:Visible"),*TunnelName));
		}
		else
		{
			SetActorLocation(BasicLoc);
			SetActorHiddenInGame(true);
			UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("OwnerTunnelName:%s:Hidden"),*TunnelName));
		}
	}
}