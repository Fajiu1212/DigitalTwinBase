#include "Actor/BaseGeologicalForecastGenerator.h"

#include "AutoParse.h"
#include "AwsaWebUI_Manager.h"
#include "AwsaWebUI_Subsystem.h"
#include "JsonLibraryValue.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Func/TunnelSectionCenterlineLibrary.h"
#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "Kismet/KismetMathLibrary.h"


ABaseGeologicalForecastGenerator::ABaseGeologicalForecastGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;
	RootComponent->SetMobility(EComponentMobility::Movable);

	CustomSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("CustomSplineComponent"));
	CustomSplineComponent->SetupAttachment(RootComponent);
	CustomSplineComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	BaseStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseStaticMeshComponent"));
	BaseStaticMeshComponent->SetupAttachment(RootComponent);
	BaseStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	BaseStaticMeshComponent->SetMaterial(0, MaterialInterfaces_Source);

	SampleDynamicMeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("DynamicMeshComponent"));
	SampleDynamicMeshComponent->SetupAttachment(RootComponent);
	SampleDynamicMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ABaseGeologicalForecastGenerator::BeginPlay()
{
	Super::BeginPlay();

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		Subsystem->OnGeologicalForecastLoadSuccess.AddDynamic(
			this, &ABaseGeologicalForecastGenerator::OnGeologicalForecastLoadSuccess);
		Subsystem->ModuleSwitched.AddDynamic(this, &ABaseGeologicalForecastGenerator::OnModuleSwitched);
		Subsystem->OnOverExcavationLoadSuccess.AddDynamic(
			this, &ABaseGeologicalForecastGenerator::OnOverExcavationLoadSuccess);
		Subsystem->OnSurfaceInfoLoadSuccess.AddDynamic(this, &ABaseGeologicalForecastGenerator::OnSurfaceLoadSuccess);
	}
	if (IsValid(SampleDynamicMeshComponent))
	{
		SampleDynamicMeshComponent->SetVisibility(false);
	}
}

void ABaseGeologicalForecastGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ABaseGeologicalForecastGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ABaseGeologicalForecastGenerator::MouseOverlay_Component_Implementation(UMeshComponent* TargetComponent)
{
	IZHJZ_Interface::MouseOverlay_Component_Implementation(TargetComponent);
}

void ABaseGeologicalForecastGenerator::MouseLeave_Component_Implementation(UMeshComponent* TargetComponent)
{
	IZHJZ_Interface::MouseLeave_Component_Implementation(TargetComponent);
}

void ABaseGeologicalForecastGenerator::MouseClick_Component_Implementation(UMeshComponent* TargetComponent)
{
	IZHJZ_Interface::MouseClick_Component_Implementation(TargetComponent);

	int32 OutId = 0;
	if (TryExtractGeologicalForecastId(TargetComponent, OutId))
	{
		if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
		{
			FJsonLibraryValue Data = FJsonLibraryValue(FString::Printf(
				TEXT("{\"component\":\"advancedGeologicalPrediction\",\"params\":{\"id\":\"%d\"}}"), OutId));
			//FString DataStr = FString::Printf(TEXT("{\"component\":\"advancedGeologicalPrediction\",\"params\":{\"id\":\"%d\"}}"),OutId);
			Subsystem->CallWebFunc(TEXT("getCurrentSection"), FJsonLibraryValue(OutId));
			Subsystem->CallWebFunc(TEXT("setCenterPopup"), FJsonLibraryValue(Data));
		}
	}
}

#if WITH_EDITOR
void ABaseGeologicalForecastGenerator::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(
			ABaseGeologicalForecastGenerator, BaseStaticMesh))
	{
		if (!BaseStaticMesh || !BaseStaticMeshComponent)
		{
			return;
		}
		BaseStaticMeshComponent->SetStaticMesh(BaseStaticMesh);
		if (GEditor)
		{
			GEditor->NoteSelectionChange();
			GEditor->RedrawAllViewports();
		}
	}
}
#endif

UDynamicMesh* ABaseGeologicalForecastGenerator::CopyMeshFromSourceStaticMesh(UDynamicMeshComponent* TargetComponent,
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
		UAutoParse::PrintLog_GameThread(TEXT("CopyStaticMeshToDynamicMesh: TargetComponent is null"));
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

void ABaseGeologicalForecastGenerator::RebuildSpline()
{
	if (bUseCustomSpline)
	{
		return;
	}
	ensure(IsInGameThread());
	if (IsValid(SplineComponent))
	{
#if WITH_EDITOR
		RemoveInstanceComponent(SplineComponent);
#endif
		SplineComponent->DestroyComponent();
		SplineComponent = nullptr;
	}
	UDynamicMesh* SampleDynamicMesh = CopyMeshFromSourceStaticMesh(SampleDynamicMeshComponent, true);
	SplineComponent = UTunnelSectionCenterlineLibrary::BuildSimpleCenterSpline(
		this, SampleDynamicMesh, RootComponent,
		1000, 512, false, FVector::ZeroVector, FVector::ZeroVector,
		NAME_None, true, true, 1000, false, 5,
		true, 0.1, 1000);


	if (IsValid(SampleDynamicMeshComponent))
	{
		SampleDynamicMeshComponent->SetVisibility(false);
	}
#if WITH_EDITOR
	Modify();
#endif
}

void ABaseGeologicalForecastGenerator::AddDM()
{
	for (auto CreatedDM : CutDynamicMeshComponents)
	{
		if (IsValid(CreatedDM))
		{
#if WITH_EDITOR
			RemoveInstanceComponent(CreatedDM);
#endif
			CreatedDM->DestroyComponent();
			CreatedDM = nullptr;
		}
	}
	CutDynamicMeshComponents.Empty();
	AddCutDynamicMeshComponents(0, 0.33, 0, 1);
	AddCutDynamicMeshComponents(0.33, 0.66, 1, 2);
	AddCutDynamicMeshComponents(0.66, 1, 2, 3);
}

void ABaseGeologicalForecastGenerator::CutDynamicMesh()
{
}

void ABaseGeologicalForecastGenerator::OnGeologicalForecastLoadSuccess()
{
	for (auto CreatedDM : CutDynamicMeshComponents)
	{
		if (IsValid(CreatedDM))
		{
#if WITH_EDITOR
			RemoveInstanceComponent(CreatedDM);
#endif
			CreatedDM->DestroyComponent();
			CreatedDM = nullptr;
		}
	}
	CutDynamicMeshComponents.Empty();
	CutMiddlePercents.Empty();
	UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>();
	if (!Subsystem)
	{
		return;
	}

	Subsystem->FindGeologicalForecastByTunnelName(TunnelName, GeologicalForecastInfo);
	for (const auto& CreatedDM : GeologicalForecastInfo.GeologicalForecastArr)
	{
		float BeginPercent = CreatedDM.BeginMileage == 0 ? 0.f : CreatedDM.BeginMileage / TotalTunnelLength;
		float EndPercent = CreatedDM.EndMileage == 0 ? 0.f : CreatedDM.EndMileage / TotalTunnelLength;
		AddCutDynamicMeshComponents(BeginPercent, EndPercent, CreatedDM.LevelIndex, CreatedDM.ID);
		CutMiddlePercents.Add(FCutCenterInfo((BeginPercent + EndPercent) / 2.f, CreatedDM.LevelIndex));
	}
	// SetActorHiddenInGame(false);
	// SetActorEnableCollision(true);
}

UDynamicMeshComponent* ABaseGeologicalForecastGenerator::CreateDynamicMeshComponent()
{
	UDynamicMeshComponent* NewComp = NewObject<UDynamicMeshComponent>(this, NAME_None, RF_Transactional);
	if (!NewComp)
	{
		return nullptr;
	}
	CopyMeshFromSourceStaticMesh(NewComp, true);

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

void ABaseGeologicalForecastGenerator::AddCutDynamicMeshComponents(const float BeginPercent, const float EndPercent,
                                                                   int32 MtlLevel, int32 ID)
{
	USplineComponent* ActiveSplineComponent = bUseCustomSpline ? CustomSplineComponent : SplineComponent;
	if (!ActiveSplineComponent)
	{
		return;
	}

	UDynamicMeshComponent* NewDynamicMesh = CreateDynamicMeshComponent();
	if (NewDynamicMesh)
	{
		CutDynamicMeshComponents.Emplace(NewDynamicMesh);
#if WITH_EDITOR
		NewDynamicMesh->SetFlags(RF_Transactional);
#endif
	}

	float FirstPercent = bFlip ? 1.0f - BeginPercent : BeginPercent;
	float SecondPercent = bFlip ? 1.0f - EndPercent : EndPercent;
	// First cut
	FVector Location0 = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
		ActiveSplineComponent->GetSplineLength() *
		FirstPercent,
		ESplineCoordinateSpace::Local);
	FVector Direction0 = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
		ActiveSplineComponent->GetSplineLength() * FirstPercent,
		ESplineCoordinateSpace::Local);
	FRotator Rotation0 = FRotationMatrix::MakeFromZ(Direction0).Rotator();

	FGeometryScriptMeshPlaneCutOptions Option0;
	Option0.bFlipCutSide = !bFlip;
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
		NewDynamicMesh->GetDynamicMesh(), UKismetMathLibrary::MakeTransform(Location0, Rotation0), Option0);

	// Sscond cut
	FVector Location1 = ActiveSplineComponent->GetLocationAtDistanceAlongSpline(
		ActiveSplineComponent->GetSplineLength() *
		SecondPercent,
		ESplineCoordinateSpace::Local);
	FVector Direction1 = ActiveSplineComponent->GetDirectionAtDistanceAlongSpline(
		ActiveSplineComponent->GetSplineLength() * SecondPercent,
		ESplineCoordinateSpace::Local);
	FRotator Rotation1 = FRotationMatrix::MakeFromZ(Direction1).Rotator();

	FGeometryScriptMeshPlaneCutOptions Option1;
	Option1.bFlipCutSide = bFlip;
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
		NewDynamicMesh->GetDynamicMesh(), UKismetMathLibrary::MakeTransform(Location1, Rotation1), Option1);

	SetDynamicMeshMaterial(NewDynamicMesh, MtlLevel);


	NewDynamicMesh->SetCollisionObjectType(ECC_GameTraceChannel1);
	NewDynamicMesh->SetComplexAsSimpleCollisionEnabled(true, true);
	NewDynamicMesh->ComponentTags.Add(FName(*FString::Printf(TEXT("GeologicalForecast_%d"), ID)));

	// TO DO : Fix hard core
	if (TunnelName.Equals(TEXT("3#支洞"), ESearchCase::IgnoreCase))
	{
		NewDynamicMesh->SetHiddenInGame(false);
		NewDynamicMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	}
	else
	{
		NewDynamicMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		NewDynamicMesh->SetHiddenInGame(true);
	}
}

void ABaseGeologicalForecastGenerator::SetDynamicMeshMaterial(UDynamicMeshComponent* TargetDynamicMesh,
                                                              int32 MaterialIndex)
{
	int32 MtlIndex = MaterialIndex - 1;
	if (IsValid(TargetDynamicMesh) && MaterialInterfaces_Levels.IsValidIndex(MtlIndex))
	{
		TargetDynamicMesh->SetMaterial(0, MaterialInterfaces_Levels[MtlIndex]);
		TargetDynamicMesh->MarkRenderStateDirty();
	}
#if WITH_EDITOR
	Modify();
#endif
}

bool ABaseGeologicalForecastGenerator::TryExtractGeologicalForecastId(const UActorComponent* Component, int32& OutId)
{
	OutId = INDEX_NONE;
	if (!IsValid(Component))
	{
		return false;
	}

	static const FString Prefix = TEXT("GeologicalForecast_");
	for (const FName& TagName : Component->ComponentTags)
	{
		const FString TagStr = TagName.ToString();
		if (TagStr.StartsWith(Prefix))
		{
			const FString Suffix = TagStr.Mid(Prefix.Len());
			if (Suffix.IsNumeric())
			{
				OutId = FCString::Atoi(*Suffix);
				return true;
			}
		}
	}
	return false;
}

void ABaseGeologicalForecastGenerator::OnModuleSwitched(const FString& InModuleName, const FString& InExtraParam)
{
	if (InModuleName.Equals(TEXT("超前地质预报"), ESearchCase::CaseSensitive))
	{
		if (bIsSlidesShown)
		{
			ShowSlides(false);
			bIsSlidesShown = false;
		}
		if (bIsSurfaceShown)
		{
			ShowSurfaces(false);
			bIsSurfaceShown = false;
		}
		if (InExtraParam == TunnelName)
		{
			SetActorHiddenInGame(false);
			SetActorEnableCollision(true);
			ShowCenterWidget(true);
			bIsGeoWidgetVisible = true;
			for (auto DM : CutDynamicMeshComponents)
			{
				if (IsValid(DM))
				{
					DM->SetHiddenInGame(false);
					DM->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
				}
			}
		}
		else
		{
			SetActorHiddenInGame(false);
			// SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
			ShowCenterWidget(false);
			bIsGeoWidgetVisible = false;
			for (auto DM : CutDynamicMeshComponents)
			{
				if (IsValid(DM))
				{
					DM->SetHiddenInGame(true);
					DM->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				}
			}
		}
	}
	else if (InModuleName.Equals(TEXT("隧道超欠挖"), ESearchCase::CaseSensitive))
	{
		if (bIsSurfaceShown)
		{
			ShowSurfaces(false);
			bIsSurfaceShown = false;
		}
		
		if (bIsGeoWidgetVisible)
		{
			ShowCenterWidget(false);
			bIsGeoWidgetVisible = false;
		}
		
		if (InExtraParam == TunnelName)
		{
			SetActorHiddenInGame(false);
			SetActorEnableCollision(false);
			for (auto DM : CutDynamicMeshComponents)
			{
				if (IsValid(DM))
				{
					DM->SetHiddenInGame(true);
					DM->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				}
			}
			bIsSlidesShown = true;
			ShowSlides(true);
		}
		else
		{
			SetActorHiddenInGame(false);
			// SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
			for (auto DM : CutDynamicMeshComponents)
			{
				if (IsValid(DM))
				{
					DM->SetHiddenInGame(true);
					DM->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				}
			}
			if (bIsSlidesShown)
			{
				ShowSlides(false);
				bIsSlidesShown = false;
			}
		}
	}
	else if (InModuleName.Equals(TEXT("围岩变形监测"), ESearchCase::CaseSensitive))
	{
		if (bIsSlidesShown)
		{
			ShowSlides(false);
			bIsSlidesShown = false;
		}
		
		if (bIsGeoWidgetVisible)
		{
			ShowCenterWidget(false);
			bIsGeoWidgetVisible = false;
		}
		
		if (InExtraParam == TunnelName)
		{
			SetActorHiddenInGame(false);
			SetActorEnableCollision(false);
			for (auto DM : CutDynamicMeshComponents)
			{
				if (IsValid(DM))
				{
					DM->SetHiddenInGame(true);
					DM->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				}
			}
			bIsSurfaceShown = true;
			ShowSurfaces(true);
		}
		else
		{
			SetActorHiddenInGame(false);
			// SetActorHiddenInGame(true);
			SetActorEnableCollision(false);
			for (auto DM : CutDynamicMeshComponents)
			{
				if (IsValid(DM))
				{
					DM->SetHiddenInGame(true);
					DM->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				}
			}
			if (bIsSurfaceShown)
			{
				ShowSurfaces(false);
				bIsSurfaceShown = false;
			}
		}
	}
	else
	{
		if (bIsSurfaceShown)
		{
			ShowSurfaces(false);
			bIsSurfaceShown = false;
		}
		if (bIsSlidesShown)
		{
			ShowSlides(false);
			bIsSlidesShown = false;
		}
		
		if (bIsGeoWidgetVisible)
		{
			ShowCenterWidget(false);
			bIsGeoWidgetVisible = false;
		}
		
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void ABaseGeologicalForecastGenerator::OnOverExcavationLoadSuccess()
{
	SlidesArr.Reset();
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		Subsystem->FindSlidesArrByTunnelName(TunnelName, SlidesArr);
	}
}

void ABaseGeologicalForecastGenerator::OnSurfaceLoadSuccess()
{
	SurfacesArr.Reset();
	if (UAwsaWebUI_Subsystem* Subsystem = GetGameInstance()->GetSubsystem<UAwsaWebUI_Subsystem>())
	{
		Subsystem->FindSurfaceArrByTunnelName(TunnelName, SurfacesArr);
	}
}
