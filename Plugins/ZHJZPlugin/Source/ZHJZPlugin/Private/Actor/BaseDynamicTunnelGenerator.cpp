#include "Actor/BaseDynamicTunnelGenerator.h"

#include "AutoParse.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "DynamicMeshActor.h"
#include "JsonLibraryValue.h"
#include "Func/TunnelSectionCenterlineLibrary.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshRepairFunctions.h"
#include "Kismet/KismetMathLibrary.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

ABaseDynamicTunnelGenerator::ABaseDynamicTunnelGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	RootComponent->SetMobility(EComponentMobility::Movable);
	CustomSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("CustomSplineComponent"));
	CustomSplineComponent->SetupAttachment(RootComponent);

	DynamicMeshComponent_0 = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMeshComponent_0"));
	DynamicMeshComponent_0->SetupAttachment(RootComponent);

	DynamicMeshComponent_1 = CreateDefaultSubobject<UDynamicMeshComponent>(
		TEXT("DynamicMeshComponent_1"));
	DynamicMeshComponent_1->SetupAttachment(RootComponent);
	DynamicMeshComponent_2 = CreateDefaultSubobject<UDynamicMeshComponent>(
		TEXT("DynamicMeshComponent_2"));
	DynamicMeshComponent_2->SetupAttachment(RootComponent);
}

void ABaseDynamicTunnelGenerator::BeginPlay()
{
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		Subsystem->OnProgressLoadSuccess.AddDynamic(this, &ABaseDynamicTunnelGenerator::OnProgressLoadSuccess);
		Subsystem->ModuleSwitched.AddDynamic(this, &ABaseDynamicTunnelGenerator::OnModuleSwitched);
		Subsystem->OnExplosionAnimation.AddDynamic(this, &ABaseDynamicTunnelGenerator::OnExplosionAnimation);
		Subsystem->OnSectionCuttingSelected.AddDynamic(this, &ABaseDynamicTunnelGenerator::OnSectionCuttingSelected);
	}
	BasicLoc = GetActorLocation();

	Super::BeginPlay();
}

void ABaseDynamicTunnelGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		Subsystem->OnProgressLoadSuccess.RemoveDynamic(this, &ABaseDynamicTunnelGenerator::OnProgressLoadSuccess);
		Subsystem->ModuleSwitched.RemoveDynamic(this, &ABaseDynamicTunnelGenerator::OnModuleSwitched);
		Subsystem->OnExplosionAnimation.RemoveDynamic(this, &ABaseDynamicTunnelGenerator::OnExplosionAnimation);
		Subsystem->OnSectionCuttingSelected.RemoveDynamic(this, &ABaseDynamicTunnelGenerator::OnSectionCuttingSelected);
	}
	Super::EndPlay(EndPlayReason);
}

void ABaseDynamicTunnelGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//EnsureDynamicMeshComponents();

	// if (!DynamicMeshComponents.IsEmpty())
	// {
	// 	DynamicMeshComponents[0]->SetHiddenInGame(false);
	// }


	// if (RootComponent)
	// {
	// 	TArray<USceneComponent*> ClearChildren = RootComponent->GetAttachChildren();
	// 	for (USceneComponent* Child : ClearChildren)
	// 	{
	// 		if (!Child)
	// 		{
	// 			continue;
	// 		}
	//
	// 		const bool bIsKept =
	// 			Child == CustomSplineComponent ||
	// 			Child == SplineComponent ||
	// 			Child == DynamicMeshComponent_0 ||
	// 			Child == DynamicMeshComponent_1 ||
	// 			Child == DynamicMeshComponent_2;
	//
	// 		if (!bIsKept && Child->GetAttachParent() == RootComponent)
	// 		{
	// 			Child->DestroyComponent();
	// 		}
	// 	}
	// }
}

#if WITH_EDITOR
void ABaseDynamicTunnelGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ABaseDynamicTunnelGenerator::MouseClick_Implementation(bool isBool)
{
	IZHJZ_Interface::MouseClick_Implementation(isBool);

	if (UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		WebUI_Subsystem->CallOnExplosionAnimation(OwnerTunnelName, true);
		WebUI_Subsystem->CallWebFunc(TEXT("progressMgtDetail"), FJsonLibraryValue(OwnerTunnelName));
	}
}


UDynamicMeshComponent* ABaseDynamicTunnelGenerator::CreateDynamicMeshComponent(int32 Index)
{
	const FName CompName = *FString::Printf(TEXT("DynamicMesh_%d"), Index + 1);

	UDynamicMeshComponent* NewComp = NewObject<UDynamicMeshComponent>(this, CompName, RF_Transactional);
	if (!NewComp)
	{
		return nullptr;
	}

#if WITH_EDITOR
	NewComp->CreationMethod = EComponentCreationMethod::Instance;
#endif

	AddInstanceComponent(NewComp);

	if (RootComponent)
	{
		NewComp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	else
	{
		NewComp->SetupAttachment(nullptr);
	}

	NewComp->RegisterComponent();
	NewComp->SetMobility(EComponentMobility::Movable);

	return NewComp;
}

void ABaseDynamicTunnelGenerator::DestroyDynamicMeshComponent(UDynamicMeshComponent* Comp)
{
	if (!Comp)
	{
		return;
	}

	RemoveInstanceComponent(Comp);

	if (Comp->IsRegistered())
	{
		Comp->UnregisterComponent();
	}

	Comp->DestroyComponent();
}

UDynamicMesh* ABaseDynamicTunnelGenerator::CopyMeshFromSourceStaticMesh(UDynamicMeshComponent* TargetComponent,
                                                                        bool bResetTargetMesh)
{
	ensure(IsInGameThread());

	if (!IsValid(SourceStaticMesh))
	{
		return nullptr;
	}
	if (TargetComponent == nullptr)
	{
		UAutoParse::PrintLog_GameThread(TEXT("CopyStaticMeshToDynamicMesh: TargetComponent is null"));
		return nullptr;
	}

	UDynamicMesh* TargetMesh = TargetComponent->GetDynamicMesh();
	if (TargetMesh == nullptr)
	{
		UAutoParse::PrintLog_GameThread(TEXT("CopyStaticMeshToDynamicMesh: TargetMesh is null"));
		return nullptr;
	}

	if (bResetTargetMesh)
	{
		TargetMesh->Reset();
	}

	FGeometryScriptCopyMeshFromAssetOptions AssetOptions;
	AssetOptions.bApplyBuildSettings = true;
	AssetOptions.bRequestTangents = true;
	AssetOptions.bIgnoreRemoveDegenerates = false;
	AssetOptions.bUseBuildScale = true;

	FGeometryScriptMeshReadLOD ReadLOD;
	ReadLOD.LODType = EGeometryScriptLODType::MaxAvailable;
	ReadLOD.LODIndex = 0;

	EGeometryScriptOutcomePins Outcome = EGeometryScriptOutcomePins::Failure;
	UGeometryScriptDebug* Debug = nullptr;

	UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(
		SourceStaticMesh,
		TargetMesh,
		AssetOptions,
		ReadLOD,
		Outcome,
		Debug
	);

	if (Outcome != EGeometryScriptOutcomePins::Success)
	{
		UAutoParse::PrintLog_GameThread(FString::Printf(
			TEXT("CopyStaticMeshToDynamicMesh: CopyMeshFromStaticMeshV2 failed for asset %s"),
			*GetNameSafe(SourceStaticMesh)));
		return nullptr;
	}

	return TargetMesh;
}

void ABaseDynamicTunnelGenerator::SetDMMaterials()
{
	ensure(IsInGameThread());
	//
	// 	bool bFlipCutting = ModuleName.Equals(TEXT("剖面"), ESearchCase::Type::CaseSensitive) || ModuleName.Equals(
	// 		TEXT("FPP"), ESearchCase::Type::CaseSensitive);
	// 	if (bFlipCutting)
	// 	{
	// 		// if (UDynamicMesh** DMPtr = DynamicMeshes.Find(0))
	// 		// {
	// 		// 	if (!*DMPtr)
	// 		// 	{
	// 		// 		return;
	// 		// 	}
	// 		// }
	// 		UDynamicMeshComponent* Comp = DynamicMeshComponents.IsValidIndex(0) ? DynamicMeshComponents[0] : nullptr;
	// 		if (!IsValid(Comp))
	// 		{
	// 			return;
	// 		}
	// 		Comp->SetMaterial(0, Material_Completed);
	// 		Comp->MarkRenderStateDirty();
	// 	}
	// 	else
	// 	{
	// 		for (int32 i = 0; i < (bIsMultiple ? 3 : 2); ++i)
	// 		{
	// 			if (UDynamicMesh** DMPtr = DynamicMeshes.Find(i))
	// 			{
	// 				UDynamicMesh* DM = *DMPtr;
	// 				if (!DM)
	// 				{
	// 					continue;
	// 				}
	// 				UDynamicMeshComponent* Comp =
	// 					DynamicMeshComponents.IsValidIndex(i) ? DynamicMeshComponents[i] : nullptr;
	// 				if (!IsValid(Comp))
	// 				{
	// 					continue;
	// 				}
	// 				Comp->SetMaterial(0, i == 1 ? Material_UnFinished : Material_Completed);
	// 				Comp->MarkRenderStateDirty();
	// 			}
	// 		}
	// 	}
	//
	// #if WITH_EDITOR
	// 	Modify();
	// #endif
	if (!DynamicMeshComponent_0 || !Material_Completed
		|| !DynamicMeshComponent_1
		|| !DynamicMeshComponent_2 || !Material_UnFinished)
	{
		return;
	}
	DynamicMeshComponent_0->SetMaterial(0, Material_Completed);
	DynamicMeshComponent_1->SetMaterial(0, Material_UnFinished);
	DynamicMeshComponent_2->SetMaterial(0, Material_Completed);
}

void ABaseDynamicTunnelGenerator::BuildSpline()
{
	if (bUseCustomSpline)
	{
		return;
	}
	ensure(IsInGameThread());
	UDynamicMesh* DMPtr = OriginalDynamicMesh_0;
	if (!DMPtr || !IsValid(DMPtr) || !DynamicMeshComponent_0 || !IsValid(DynamicMeshComponent_0))
	{
		SplineComponent = nullptr;
		return;
	}
	SplineComponent = UTunnelSectionCenterlineLibrary::BuildSimpleCenterSpline(
		this, OriginalDynamicMesh_0, DynamicMeshComponent_0,
		1000, 512, false, FVector::ZeroVector, FVector::ZeroVector,
		TEXT("TunnelCenterSpline"), true, true, 1000, false, 5,
		true, 0.1, 1000);
#if WITH_EDITOR
	Modify();
#endif
}

void ABaseDynamicTunnelGenerator::CutDynamicMesh()
{
	USplineComponent* ActiveSplineComponent = SplineComponent;
	if (!ActiveSplineComponent)
	{
		return;
	}

	bool bFlipCutting = ModuleName.Equals(TEXT("剖面"), ESearchCase::Type::CaseSensitive) || ModuleName.Equals(
		TEXT("FPP"), ESearchCase::Type::CaseSensitive);
	if (bFlipCutting)
	{
		if (bIsMultiple)
		{
			if (OriginalDynamicMesh_0 && PercentArr.Num() == 2)
			{
				float p0 = PercentArr[0];
				float p1 = PercentArr[1];

				float Percent1 = bFlip ? 1.0f - p0 : p0;
				float Percent2 = bFlip ? p1 : 1.0f - p1;

				// DynamicMesh 0 && 2
				// Cut start
				UDynamicMesh* DynamicMesh = DuplicateObject<UDynamicMesh>(OriginalDynamicMesh_0, this);
				FVector Location0 = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() *
					Percent1,
					ESplineCoordinateSpace::Local);
				FVector Direction0 = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() * Percent1,
					ESplineCoordinateSpace::Local);
				FRotator Rotation0 = FRotationMatrix::MakeFromZ(Direction0).Rotator();
				FGeometryScriptMeshPlaneCutOptions Option0;
				Option0.bFillHoles = true;
				Option0.bFillSpans = true;
				Option0.bFlipCutSide = !bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh, UKismetMathLibrary::MakeTransform(Location0, Rotation0), Option0);

				// Cut end
				FVector Location2 = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() *
					Percent2,
					ESplineCoordinateSpace::Local);
				FVector Direction2 = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() * Percent2,
					ESplineCoordinateSpace::Local);
				FRotator Rotation2 = FRotationMatrix::MakeFromZ(Direction2).Rotator();
				FGeometryScriptMeshPlaneCutOptions Option2;
				Option2.bFlipCutSide = bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh, UKismetMathLibrary::MakeTransform(Location2, Rotation2), Option2);
				if (IsValid(DynamicMeshComponent_0) && DynamicMesh)
				{
					DynamicMeshComponent_0->SetDynamicMesh(DynamicMesh);
					DynamicMeshComponent_0->SetHiddenInGame(false);
					DynamicMeshComponent_0->SetCollisionObjectType(ECC_WorldStatic);
					DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
				}
				else
				{
					UAutoParse::PrintLog_GameThread(TEXT("Error"));
				}
			}
		}
		else
		{
			if (OriginalDynamicMesh_0 && PercentArr.Num() == 1)
			{
				float Percent = bFlip ? 1 - PercentArr[0] : PercentArr[0];

				if (Percent < 0.01)
				{
					UDynamicMesh* DynamicMesh = DuplicateObject<UDynamicMesh>(OriginalDynamicMesh_0, this);
					if (IsValid(DynamicMeshComponent_0) && DynamicMesh)
					{
						DynamicMeshComponent_0->SetDynamicMesh(DynamicMesh);
						DynamicMeshComponent_0->SetHiddenInGame(false);
						DynamicMeshComponent_0->SetCollisionObjectType(ECC_WorldStatic);
						DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
					}
					return;
				}
				if (Percent > 0.99)
				{
					if (IsValid(DynamicMeshComponent_0))
					{
						DynamicMeshComponent_0->SetHiddenInGame(true);
						DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
					}
					return;
				}

				UDynamicMesh* DynamicMesh = DuplicateObject<UDynamicMesh>(OriginalDynamicMesh_0, this);
				FVector Location = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() *
					Percent,
					ESplineCoordinateSpace::Local);
				FVector Direction = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() * Percent,
					ESplineCoordinateSpace::Local);
				FRotator Rotation = FRotationMatrix::MakeFromZ(Direction).Rotator();

				FGeometryScriptMeshPlaneCutOptions Option;
				Option.bFlipCutSide = !bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh, UKismetMathLibrary::MakeTransform(Location, Rotation), Option);

				if (IsValid(DynamicMeshComponent_0) && DynamicMesh)
				{
					DynamicMeshComponent_0->SetDynamicMesh(DynamicMesh);
					DynamicMeshComponent_0->SetHiddenInGame(false);
					DynamicMeshComponent_0->SetCollisionObjectType(ECC_WorldStatic);
					DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
				}
			}
		}
	}
	else
	{
		if (bIsMultiple)
		{
			if (OriginalDynamicMesh_0 && OriginalDynamicMesh_1 && OriginalDynamicMesh_2 && PercentArr.Num() == 2)
			{
				float p0 = PercentArr[0];
				float p1 = PercentArr[1];

				float Percent1 = bFlip ? 1.0f - p0 : p0;
				float Percent2 = bFlip ? p1 : 1.0f - p1;

				UDynamicMesh* DynamicMesh0 = DuplicateObject<UDynamicMesh>(OriginalDynamicMesh_0, this);
				UDynamicMesh* DynamicMesh1 = DuplicateObject<UDynamicMesh>(OriginalDynamicMesh_1, this);
				UDynamicMesh* DynamicMesh2 = DuplicateObject<UDynamicMesh>(OriginalDynamicMesh_2, this);

				// Index == 0
				FVector Location0 = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() *
					Percent1,
					ESplineCoordinateSpace::Local);
				FVector Direction0 = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() * Percent1,
					ESplineCoordinateSpace::Local);
				FRotator Rotation0 = FRotationMatrix::MakeFromZ(Direction0).Rotator();
				FGeometryScriptMeshPlaneCutOptions Option0;
				Option0.bFlipCutSide = bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh0, UKismetMathLibrary::MakeTransform(Location0, Rotation0), Option0);

				Option0.bFlipCutSide = !bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh1, UKismetMathLibrary::MakeTransform(Location0, Rotation0), Option0);

				if (IsValid(DynamicMeshComponent_0) && DynamicMesh0)
				{
					DynamicMeshComponent_0->SetDynamicMesh(DynamicMesh0);
					DynamicMeshComponent_0->SetHiddenInGame(false);
					DynamicMeshComponent_0->SetCollisionObjectType(ECC_GameTraceChannel1);
					DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
				}

				// Index == 2

				FVector Location2 = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() *
					Percent2,
					ESplineCoordinateSpace::Local);
				FVector Direction2 = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() * Percent2,
					ESplineCoordinateSpace::Local);
				FRotator Rotation2 = FRotationMatrix::MakeFromZ(Direction2).Rotator();
				FGeometryScriptMeshPlaneCutOptions Option2;
				Option2.bFlipCutSide = !bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh2, UKismetMathLibrary::MakeTransform(Location2, Rotation2), Option2);

				Option2.bFlipCutSide = bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh1, UKismetMathLibrary::MakeTransform(Location2, Rotation2), Option2);
				if (IsValid(DynamicMeshComponent_2) && DynamicMesh2)
				{
					DynamicMeshComponent_2->SetDynamicMesh(DynamicMesh2);
					DynamicMeshComponent_2->SetHiddenInGame(false);
					DynamicMeshComponent_2->SetCollisionObjectType(ECC_GameTraceChannel1);
					DynamicMeshComponent_2->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
				}

				if (IsValid(DynamicMeshComponent_1) && DynamicMesh1)
				{
					DynamicMeshComponent_1->SetDynamicMesh(DynamicMesh1);
					DynamicMeshComponent_1->SetHiddenInGame(false);
					DynamicMeshComponent_1->SetCollisionObjectType(ECC_GameTraceChannel1);
					DynamicMeshComponent_1->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
				}
			}
		}
		else
		{
			if (OriginalDynamicMesh_0 && OriginalDynamicMesh_1 && PercentArr.Num() == 1)
			{
				// Index == 1
				if (IsValid(DynamicMeshComponent_1))
				{
					DynamicMeshComponent_1->SetHiddenInGame(false);
					DynamicMeshComponent_1->SetCollisionObjectType(ECC_GameTraceChannel1);
					DynamicMeshComponent_1->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
				}

				// Index == 0
				UDynamicMesh* DynamicMesh0 = DuplicateObject<UDynamicMesh>(OriginalDynamicMesh_0, this);

				float Percent = bFlip ? 1 - PercentArr[0] : PercentArr[0];
				if (Percent < 0.01)
				{
					if (IsValid(DynamicMeshComponent_0))
					{
						DynamicMeshComponent_0->SetHiddenInGame(true);
						DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
					}
					return;
				}
				if (Percent > 0.99)
				{
					if (IsValid(DynamicMeshComponent_0) && DynamicMesh0)
					{
						DynamicMeshComponent_0->SetDynamicMesh(DynamicMesh0);
						DynamicMeshComponent_0->SetHiddenInGame(false);
						DynamicMeshComponent_0->SetCollisionObjectType(ECC_GameTraceChannel1);
						DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
					}
					return;
				}

				FVector Location = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() *
					Percent,
					ESplineCoordinateSpace::Local);
				FVector Direction = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
					ActiveSplineComponent->GetSplineLength() * Percent,
					ESplineCoordinateSpace::Local);
				FRotator Rotation = FRotationMatrix::MakeFromZ(Direction).Rotator();

				FGeometryScriptMeshPlaneCutOptions Option1;
				Option1.bFlipCutSide = bFlip;
				UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
					DynamicMesh0, UKismetMathLibrary::MakeTransform(Location, Rotation), Option1);

				if (IsValid(DynamicMeshComponent_0) && DynamicMesh0)
				{
					DynamicMeshComponent_0->SetDynamicMesh(DynamicMesh0);
					DynamicMeshComponent_0->SetHiddenInGame(false);
					DynamicMeshComponent_0->SetCollisionObjectType(ECC_GameTraceChannel1);
					DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
				}
			}
		}
	}
}

void ABaseDynamicTunnelGenerator::OnProgressLoadSuccess(const FString& TargetModuleName)
{
	if (!TargetModuleName.Equals(ModuleName, ESearchCase::CaseSensitive))
	{
		return;
	}
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		bool Result = Subsystem->CalculateTunnelModelDistance(OwnerTunnelName, OwnerLayer, OwnerSide,
		                                                      BeginWorkFaceName, EndWorkFaceName, Mileage);

		if (Result)
		{
			MileWidgetActorInfos.Empty();
			StakeWidgetActorInfos.Empty();

			if (bIsMultiple)
			{
				if (OwnerMileageDirections.Num() == 2)
				{
					PercentArr.Empty(2);
					PercentArr.Reserve(2);
					float Mileage1 = 0.0f;
					float TodayMileage1 = 0.f;
					if (Subsystem->FindMileageProgressPtr(OwnerTunnelName, OwnerLayer, OwnerSide,
					                                      OwnerMileageDirections[0], BeginWorkFaceName, Mileage1,
					                                      TodayMileage1))
					{
						PercentArr.Emplace(Mileage1 / Mileage);

						MileWidgetActorInfos.Add(FMileWidgetActorInfo(
							(bFlip ? 1 - Mileage1 / Mileage : Mileage1 / Mileage + (bFlip ? 1.f : 0.f)) / 2.f,
							FString::Printf(TEXT("%.2f"), Mileage1)));

						float MileageRemainder1 = TodayMileage1 - FMath::FloorToFloat(TodayMileage1 / 1000.0f) *
							1000.0f;
						int32 MileageY1 = FMath::FloorToInt(TodayMileage1 / 1000.0f);
						FString StakeStr1 = FString::Printf(TEXT("%d+%.3f"), MileageY1, MileageRemainder1);
						StakeWidgetActorInfos.Add(FStakeWidgetActorInfo(
							bFlip ? 1 - Mileage1 / Mileage : Mileage1 / Mileage
							, StakeStr1));
					}
					else
					{
						PercentArr.Emplace(0);
					}

					float Mileage2 = 0.0f;
					float TodayMileage2 = 0.f;
					if (Subsystem->FindMileageProgressPtr(OwnerTunnelName, OwnerLayer, OwnerSide,
					                                      OwnerMileageDirections[1], EndWorkFaceName, Mileage2,
					                                      TodayMileage2))
					{
						PercentArr.Emplace(Mileage2 / Mileage);
						MileWidgetActorInfos.Add(FMileWidgetActorInfo(
							(bFlip ? 1 - (1 - Mileage2 / Mileage) : 1 - Mileage2 / Mileage + (bFlip ? 0.f : 1.f)) / 2.f,
							FString::Printf(TEXT("%.2f"), Mileage2)));

						float MileageRemainder2 = TodayMileage2 - FMath::FloorToFloat(TodayMileage2 / 1000.0f) *
							1000.0f;
						int32 MileageY2 = FMath::FloorToInt(TodayMileage2 / 1000.0f);
						FString StakeStr2 = FString::Printf(TEXT("%d+%.3f"), MileageY2, MileageRemainder2);
						StakeWidgetActorInfos.Add(FStakeWidgetActorInfo(
							bFlip ? 1 - (1 - Mileage2) / Mileage : 1 - Mileage2 / Mileage
							, StakeStr2));
					}
					else
					{
						PercentArr.Emplace(0);
					}
				}
				else
				{
					return;
				}
			}
			else
			{
				if (OwnerMileageDirections.Num() == 1)
				{
					PercentArr.Empty(1);
					PercentArr.Reserve(1);
					float Mileage1 = 0.0f;
					float TodayMileage1 = 0.f;
					if (Subsystem->FindMileageProgressPtr(OwnerTunnelName, OwnerLayer, OwnerSide,
					                                      OwnerMileageDirections[0], BeginWorkFaceName, Mileage1,
					                                      TodayMileage1))
					{
						PercentArr.Emplace(Mileage1 / Mileage);
						MileWidgetActorInfos.Add(FMileWidgetActorInfo(
							(bFlip ? 1 - Mileage1 / Mileage : Mileage1 / Mileage + (bFlip ? 1.f : 0.f)) / 2.f,
							FString::Printf(TEXT("%.2f"), Mileage1)));

						float MileageRemainder1 = TodayMileage1 - FMath::FloorToFloat(TodayMileage1 / 1000.0f) *
							1000.0f;
						int32 MileageY1 = FMath::FloorToInt(TodayMileage1 / 1000.0f);
						FString StakeStr1 = FString::Printf(TEXT("%d+%.3f"), MileageY1, MileageRemainder1);
						StakeWidgetActorInfos.Add(FStakeWidgetActorInfo(
							bFlip ? 1 - Mileage1 / Mileage : Mileage1 / Mileage
							, StakeStr1));
					}
					else
					{
						PercentArr.Emplace(0.f);
					}
				}
				else
				{
					return;
				}
			}

			// for (auto DM : DynamicMeshComponents)
			// {
			// 	DM->SetHiddenInGame(true);
			// 	DM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// 	DM->SetCollisionObjectType(ECC_GameTraceChannel1);
			// }

			CutDynamicMesh();

			if (TargetModuleName.Equals(TEXT("剖面")))
			{
				// TO DO : flip the completed 挖掘进度 && 未完成进度
				// TO DO : Hard core
				if (OwnerTunnelName == TEXT("2#尾水洞"))
				{
					SetActorHiddenInGame(false);
					//CreateVehicle();
				}
			}
			else
			{
				SetActorHiddenInGame(false);
			}
		}
	}
}

void ABaseDynamicTunnelGenerator::OnModuleSwitched(const FString& TargetModule, const FString& InExtraParam)
{
	if (InExtraParam.Equals(ModuleName, ESearchCase::CaseSensitive))
	{
		if (TargetModule.Equals(TEXT("进度管理"), ESearchCase::CaseSensitive))
		{
			// for (auto DM : DynamicMeshComponents)
			// {
			// 	if (IsValid(DM))
			// 	{
			// 		DM->SetHiddenInGame(false);
			// 		DM->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
			// 	}
			// }
			SetActorHiddenInGame(false);
			SetActorEnableCollision(true);
		}
		else if (TargetModule.Equals(TEXT("FPP"), ESearchCase::CaseSensitive))
		{
			SetActorHiddenInGame(false);
			SetActorEnableCollision(true);
		}
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

void ABaseDynamicTunnelGenerator::OnExplosionAnimation(const FString& OwnerName, bool bMultiple)
{
	if (!ModuleName.Equals(TEXT("进度管理"), ESearchCase::CaseSensitive))
	{
		return;
	}
	if (OwnerName == TEXT("Reset"))
	{
		ShowMileWidgetActors(false);
		ShowStakeWidgetActors(false);
		if (GetWorldTimerManager().IsTimerActive(MoveTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		}
		bExplosion = false;
		SetActorLocation(BasicLoc);
		SetActorHiddenInGame(false);
		SetActorEnableCollision(true);
		return;
	}
	if (OwnerName != OwnerTunnelName)
	{
		ShowMileWidgetActors(false);
		ShowStakeWidgetActors(false);
		if (GetWorldTimerManager().IsTimerActive(MoveTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(MoveTimerHandle);
		}
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		if (bExplosion)
		{
			SetActorLocation(BasicLoc);
		}
		bExplosion = false;
		return;
	}
	SetActorHiddenInGame(false);
	MoveActorZSmoothly(true);
	SetActorEnableCollision(false);
}

void ABaseDynamicTunnelGenerator::MoveActorZSmoothly(bool bMoveUp)
{
	if (OwnerLayer == 3)
	{
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle,
		                                [this]()
		                                {
			                                ShowMileWidgetActors(true);
			                                ShowStakeWidgetActors(true);
		                                }, 2.f,
		                                false,
		                                2.f);

		return;
	}
	if (GetWorldTimerManager().IsTimerActive(MoveTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	}
	bExplosion = true;
	bIsMovingUp = bMoveUp;
	ElapsedTime = 0.0f;
	TargetZOffset = bIsMovingUp ? (3 - OwnerLayer) * MoveDistance : -((1 - OwnerLayer) * MoveDistance);

	GetWorldTimerManager().SetTimer(
		MoveTimerHandle,
		this,
		&ABaseDynamicTunnelGenerator::UpdateMove,
		0.016f,
		true
	);
}

void ABaseDynamicTunnelGenerator::UpdateMove()
{
	ElapsedTime += 0.016f;

	float Progress = FMath::Clamp(ElapsedTime / MoveDuration, 0.0f, 1.0f);

	float CurrentZOffset = FMath::Lerp(0, TargetZOffset, Progress);
	SetActorLocation(BasicLoc + FVector(0.0f, 0.0f, CurrentZOffset));

	if (Progress >= 1.0f)
	{
		SetActorLocation(BasicLoc + FVector(0.0f, 0.0f, TargetZOffset));
		// 由AwsaWebUI_Subsystem处理
		// if (UAwsaWebUI_Subsystem* WebUI_Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
		// {
		// 	WebUI_Subsystem->CallWebFunc(TEXT("OnLabelClickComplete"), FJsonLibraryValue());
		// }
		ShowMileWidgetActors(true);
		ShowStakeWidgetActors(true);
		GetWorldTimerManager().ClearTimer(MoveTimerHandle);
	}
}

void ABaseDynamicTunnelGenerator::OnSectionCuttingSelected(const FString& TargetTunnelName)
{
	if (ModuleName.Equals(TEXT("剖面")))
	{
		if (TargetTunnelName.Equals(OwnerTunnelName, ESearchCase::CaseSensitive))
		{
			SetActorLocation(BasicLoc);
			SetActorHiddenInGame(false);
			if (OwnerTunnelName == TEXT("2#导流洞"))
			{
				CreateVehicle();
			}
			UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("OwnerTunnelName:%s:Visible"), *OwnerTunnelName));
		}
		else
		{
			SetActorLocation(BasicLoc);
			SetActorHiddenInGame(true);
			if (OwnerTunnelName == TEXT("2#导流洞"))
			{
				DestroyVehicle();
			}
			UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("OwnerTunnelName:%s:Hidden"), *OwnerTunnelName));
		}
	}
}

void ABaseDynamicTunnelGenerator::RebuildDynamicMesh()
{
	if (bIsUpdatingComponents)
	{
		return;
	}
	bIsUpdatingComponents = true;
	ensure(IsInGameThread());

	if (!IsValid(SourceStaticMesh))
	{
		bIsUpdatingComponents = false;
		return;
	}

	// DynamicMeshes.Empty(DynamicMeshComponents.Num());
	// DynamicMeshes.Reserve(DynamicMeshComponents.Num());

	OriginalDynamicMesh_0 = CopyMeshFromSourceStaticMesh(DynamicMeshComponent_0, true);
	OriginalDynamicMesh_1 = CopyMeshFromSourceStaticMesh(DynamicMeshComponent_1,
	                                                     true);
	OriginalDynamicMesh_2 = CopyMeshFromSourceStaticMesh(DynamicMeshComponent_2,
	                                                     true);

	if (OriginalDynamicMesh_0 && OriginalDynamicMesh_1 && OriginalDynamicMesh_2)
	{
		bool bFlipCutting = ModuleName.Equals(TEXT("剖面"), ESearchCase::Type::CaseSensitive) || ModuleName.
			Equals(
				TEXT("FPP"), ESearchCase::Type::CaseSensitive);

		DynamicMeshComponent_0->SetComplexAsSimpleCollisionEnabled(true, true);
		DynamicMeshComponent_0->SetCollisionObjectType(bFlipCutting ? ECC_WorldStatic : ECC_GameTraceChannel1);
		DynamicMeshComponent_0->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		DynamicMeshComponent_0->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
		DynamicMeshComponent_0->SetHiddenInGame(true);
		DynamicMeshComponent_1->SetComplexAsSimpleCollisionEnabled(true, true);
		DynamicMeshComponent_1->SetCollisionObjectType(bFlipCutting ? ECC_WorldStatic : ECC_GameTraceChannel1);
		DynamicMeshComponent_1->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		DynamicMeshComponent_1->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
		DynamicMeshComponent_1->SetHiddenInGame(true);
		DynamicMeshComponent_2->SetComplexAsSimpleCollisionEnabled(true, true);
		DynamicMeshComponent_2->SetCollisionObjectType(bFlipCutting ? ECC_WorldStatic : ECC_GameTraceChannel1);
		DynamicMeshComponent_2->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		DynamicMeshComponent_2->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
		DynamicMeshComponent_2->SetHiddenInGame(true);
	}

	bIsUpdatingComponents = false;

	SetDMMaterials();
}

// void ABaseDynamicTunnelGenerator::EnsureDynamicMeshComponents()
// {
// 	ensure(IsInGameThread());
//
// 	bool bFlipCutting = ModuleName.Equals(TEXT("剖面"), ESearchCase::Type::CaseSensitive) || ModuleName.Equals(
// 		TEXT("FPP"), ESearchCase::Type::CaseSensitive);
// 	const int32 Target = bFlipCutting ? 1 : bIsMultiple ? 3 : 2;
//
// 	while (DynamicMeshComponents.Num() > Target)
// 	{
// 		UDynamicMeshComponent* ToDestroy = DynamicMeshComponents.Pop();
// 		if (IsValid(ToDestroy))
// 		{
// 			DestroyDynamicMeshComponent(ToDestroy);
// 		}
// 	}
//
// 	int32 StartIndex = DynamicMeshComponents.Num();
// 	for (int32 i = StartIndex; i < Target; ++i)
// 	{
// 		UDynamicMeshComponent* NewComp = CreateDynamicMeshComponent(i);
// 		if (NewComp)
// 		{
// #if WITH_EDITOR
// 			NewComp->SetFlags(RF_Transactional);
// #endif
// 			DynamicMeshComponents.Add(NewComp);
// 		}
// 	}
// #if WITH_EDITOR
// 	Modify();
// 	RerunConstructionScripts();
// #endif
// }
