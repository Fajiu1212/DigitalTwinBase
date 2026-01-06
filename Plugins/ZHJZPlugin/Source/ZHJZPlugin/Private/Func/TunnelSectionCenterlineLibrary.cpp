#include "Func/TunnelSectionCenterlineLibrary.h"
#include "AutoParse.h"

using namespace UE::Geometry;

USplineComponent* UTunnelSectionCenterlineLibrary::BuildSimpleCenterSpline(
	AActor* OwnerActor,
	UDynamicMesh* DynamicMesh,
	USceneComponent* MeshComponent,
	float StepDistance,
	int32 MaxSteps,
	bool InitialUseCustom,
	FVector InitialOriginWorld,
	FVector InitialDirectionWorld,
	FName SplineName,
	bool bReplaceExisting,
	bool bUniformResample,
	float ResampleSpacing,
	bool bDebugDraw,
	float DebugTime,
	bool bExtendToOpenings,
	float ExtensionStepRatio,
	int32 MaxExtensionSteps)
{
	if (!OwnerActor || !DynamicMesh || !MeshComponent)
	{
		UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("[SimpleCenterline] Invalid input.")));
		return nullptr;
	}

	TArray<FVector> PosLocal;
	TArray<FIntVector> Tris;
	if (!ExtractMeshCompact(DynamicMesh, PosLocal, Tris) || PosLocal.Num() < 3)
	{
		UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("[SimpleCenterline] Mesh extraction failed.")));
		return nullptr;
	}
	const FBox LocalBounds(PosLocal);

	const FTransform MeshToWorld = MeshComponent->GetComponentTransform();
	const FTransform WorldToMesh = MeshToWorld.Inverse();

	FVector OriginLocal, DirLocal;

	if (InitialUseCustom)
	{
		OriginLocal = WorldToMesh.TransformPosition(InitialOriginWorld);
		DirLocal = WorldToMesh.TransformVectorNoScale(InitialDirectionWorld).GetSafeNormal();
		if (DirLocal.IsNearlyZero())
		{
			UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("[SimpleCenterline] Invalid custom direction.")));
			return nullptr;
		}
		UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("[SimpleCenterline] Custom mode: origin=(%s), dir=(%s)"),
		                                                *OriginLocal.ToString(), *DirLocal.ToString()));
	}
	else
	{
		DirLocal = ComputePCA(PosLocal);
		if (DirLocal.IsNearlyZero())
		{
			DirLocal = FVector::ForwardVector;
		}

		const FVector MeshCenter = LocalBounds.GetCenter();
		const float SearchStep = StepDistance * 0.1f;

		UAutoParse::PrintLog_GameThread(FString::Printf(
			TEXT("[SimpleCenterline] Auto mode: mesh center=(%s), PCA dir=(%s)"),
			*MeshCenter.ToString(), *DirLocal.ToString()));

		TArray<FVector> InitialCentroids;
		const int32 NumInitialSamples = 10;
		InitialCentroids.Reserve(NumInitialSamples * 2);

		FVector BestStartOrigin;
		bool bFoundValidStart = false;

		FVector TestOrigin = MeshCenter;
		for (int32 search = 0; search < 256; ++search)
		{
			FVector TestCentroid;
			if (ComputePlaneSectionCentroid(PosLocal, Tris, TestOrigin, DirLocal, TestCentroid))
			{
				if (LocalBounds.IsInside(TestCentroid))
				{
					const float DistToCenter = FVector::Dist(TestCentroid, MeshCenter);
					const float MaxReasonableDist = LocalBounds.GetExtent().Size();

					if (DistToCenter < MaxReasonableDist)
					{
						InitialCentroids.Add(TestCentroid);
						if (!bFoundValidStart)
						{
							BestStartOrigin = TestOrigin;
							bFoundValidStart = true;
						}

						if (InitialCentroids.Num() >= NumInitialSamples)
						{
							break;
						}
					}
				}
			}
			TestOrigin -= DirLocal * SearchStep;
		}

		if (InitialCentroids.Num() < NumInitialSamples)
		{
			TestOrigin = MeshCenter;
			for (int32 search = 0; search < 256 && InitialCentroids.Num() < NumInitialSamples; ++search)
			{
				TestOrigin += DirLocal * SearchStep;
				FVector TestCentroid;
				if (ComputePlaneSectionCentroid(PosLocal, Tris, TestOrigin, DirLocal, TestCentroid))
				{
					if (LocalBounds.IsInside(TestCentroid))
					{
						const float DistToCenter = FVector::Dist(TestCentroid, MeshCenter);
						if (DistToCenter < LocalBounds.GetExtent().Size())
						{
							InitialCentroids.Add(TestCentroid);
						}
					}
				}
			}
		}

		if (InitialCentroids.Num() < 3)
		{
			UAutoParse::PrintLog_GameThread(FString::Printf(TEXT(
				"[SimpleCenterline] Not enough initial centroids (%d). Mesh may not be suitable or try custom mode."
			),
			                                                InitialCentroids.Num()));
			return nullptr;
		}

		UAutoParse::PrintLog_GameThread(
			FString::Printf(TEXT("[SimpleCenterline] Found %d initial centroids."), InitialCentroids.Num()));

		if (InitialCentroids.Num() >= 2)
		{
			InitialCentroids.Sort([&](const FVector& A, const FVector& B)
			{
				return FVector::DotProduct(A, DirLocal) < FVector::DotProduct(B, DirLocal);
			});

			FVector FittedDir = (InitialCentroids.Last() - InitialCentroids[0]);
			if (FittedDir.Normalize())
			{
				DirLocal = (DirLocal * 0.3f + FittedDir * 0.7f).GetSafeNormal();
				UAutoParse::PrintLog_GameThread(
					FString::Printf(TEXT("[SimpleCenterline] Refined direction: (%s)"), *DirLocal.ToString()));
			}
		}

		if (bFoundValidStart)
		{
			OriginLocal = BestStartOrigin;
		}
		else
		{
			OriginLocal = InitialCentroids[0] - DirLocal * (StepDistance * 0.5f);
		}

		UAutoParse::PrintLog_GameThread(
			FString::Printf(TEXT("[SimpleCenterline] Start origin: (%s)"), *OriginLocal.ToString()));
	}

	TArray<FVector> CentersLocal = TraceCentroids(
		PosLocal, Tris, LocalBounds, OriginLocal, DirLocal,
		FMath::Max(1.f, StepDistance), FMath::Clamp(MaxSteps, 1, 100000));

	if (CentersLocal.Num() < 2)
	{
		UAutoParse::PrintLog_GameThread(
			FString::Printf(TEXT("[SimpleCenterline] Only %d centers found."), CentersLocal.Num()));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("[SimpleCenterline] Traced %d center points before extension."), CentersLocal.Num());

	if (bExtendToOpenings && CentersLocal.Num() >= 2)
	{
		const float ExtStep = StepDistance * FMath::Clamp(ExtensionStepRatio, 0.05f, 0.5f);

		FVector StartDir = (CentersLocal[0] - CentersLocal[1]).GetSafeNormal();
		TArray<FVector> StartExtension;
		int32 StartExtCount = ExtendToOpeningAggressive(PosLocal, Tris, LocalBounds, CentersLocal[0], StartDir, ExtStep,
		                                                MaxExtensionSteps, StartExtension);
		if (StartExtCount > 0)
		{
			UAutoParse::PrintLog_GameThread(
				FString::Printf(TEXT("[SimpleCenterline] Extended start by %d points."), StartExtension.Num()));
			Algo::Reverse(StartExtension);
			CentersLocal.Insert(StartExtension, 0);
		}

		FVector EndDir = (CentersLocal.Last() - CentersLocal[CentersLocal.Num() - 2]).GetSafeNormal();
		TArray<FVector> EndExtension;
		int32 EndExtCount = ExtendToOpeningAggressive(PosLocal, Tris, LocalBounds, CentersLocal.Last(), EndDir, ExtStep,
		                                              MaxExtensionSteps, EndExtension);
		if (EndExtCount > 0)
		{
			UAutoParse::PrintLog_GameThread(
				FString::Printf(TEXT("[SimpleCenterline] Extended end by %d points."), EndExtension.Num()));
			CentersLocal.Append(EndExtension);
		}
	}

	UAutoParse::PrintLog_GameThread(
		FString::Printf(TEXT("[SimpleCenterline] Final %d center points (with extensions)."), CentersLocal.Num()));

	TArray<FVector> CentersWorld;
	CentersWorld.Reserve(CentersLocal.Num());
	for (const FVector& C : CentersLocal)
	{
		CentersWorld.Add(MeshToWorld.TransformPosition(C));
	}

	TArray<FVector> FinalWorld = CentersWorld;
	if (bUniformResample && CentersWorld.Num() >= 2 && ResampleSpacing > KINDA_SMALL_NUMBER)
	{
		ResampleUniform(CentersWorld, ResampleSpacing, FinalWorld);
	}

	if (bDebugDraw && OwnerActor->GetWorld())
	{
		DrawDebugPolyline(OwnerActor->GetWorld(), FinalWorld, FColor::Cyan, DebugTime);
		if (FinalWorld.Num() > 0)
		{
			DrawDebugSphere(OwnerActor->GetWorld(), FinalWorld[0], 40.f, 16, FColor::Green, false, DebugTime, 0, 4.f);
			DrawDebugString(OwnerActor->GetWorld(), FinalWorld[0] + FVector(0, 0, 50), TEXT("START"), nullptr,
			                FColor::Green, DebugTime, true, 1.5f);
		}
		if (FinalWorld.Num() > 1)
		{
			DrawDebugSphere(OwnerActor->GetWorld(), FinalWorld.Last(), 40.f, 16, FColor::Red, false, DebugTime, 0, 4.f);
			DrawDebugString(OwnerActor->GetWorld(), FinalWorld.Last() + FVector(0, 0, 50), TEXT("END"), nullptr,
			                FColor::Red, DebugTime, true, 1.5f);
		}
		for (int32 i = 0; i < FMath::Min(3, FinalWorld.Num()); ++i)
		{
			DrawDebugSphere(OwnerActor->GetWorld(), FinalWorld[i], 15.f, 12, FColor::Yellow, false, DebugTime, 0, 2.f);
		}
	}

	USplineComponent* Spline = CreateOrFindSpline(OwnerActor, SplineName, bReplaceExisting);
	if (!Spline)
	{
		return nullptr;
	}
	WriteWorldToSpline(Spline, FinalWorld);
	return Spline;
}

bool UTunnelSectionCenterlineLibrary::ExtractMeshCompact(UDynamicMesh* DynMesh, TArray<FVector>& OutPos,
                                                         TArray<FIntVector>& OutTris)
{
	OutPos.Reset();
	OutTris.Reset();
	if (!DynMesh)
	{
		return false;
	}

	bool bOK = true;
	DynMesh->ProcessMesh([&](const FDynamicMesh3& Mesh)
	{
		TMap<int32, int32> VidToIdx;
		OutPos.Reserve(Mesh.VertexCount());
		for (int32 vid : Mesh.VertexIndicesItr())
		{
			const int32 idx = OutPos.Num();
			VidToIdx.Add(vid, idx);
			OutPos.Add((FVector)Mesh.GetVertex(vid));
		}
		OutTris.Reserve(Mesh.TriangleCount());
		for (int32 tid : Mesh.TriangleIndicesItr())
		{
			const FIndex3i t = Mesh.GetTriangle(tid);
			OutTris.Add(FIntVector(VidToIdx[t.A], VidToIdx[t.B], VidToIdx[t.C]));
		}
	});

	return bOK && OutPos.Num() > 0 && OutTris.Num() > 0;
}

static FVector PowerIteratePCA(const double C[3][3], int iters = 32)
{
	FVector v(1, 0, 0);
	for (int i = 0; i < iters; ++i)
	{
		FVector w(
			C[0][0] * v.X + C[0][1] * v.Y + C[0][2] * v.Z,
			C[1][0] * v.X + C[1][1] * v.Y + C[1][2] * v.Z,
			C[2][0] * v.X + C[2][1] * v.Y + C[2][2] * v.Z
		);
		if (!w.Normalize())
		{
			break;
		}
		v = w;
	}
	v.Normalize();
	return v;
}

FVector UTunnelSectionCenterlineLibrary::ComputePCA(const TArray<FVector>& Pos)
{
	if (Pos.Num() < 2)
	{
		return FVector::ForwardVector;
	}
	FVector Mean(0, 0, 0);
	for (const FVector& p : Pos)
	{
		Mean += p;
	}
	Mean /= (float)Pos.Num();

	double C[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
	for (const FVector& p : Pos)
	{
		const FVector d = p - Mean;
		C[0][0] += d.X * d.X;
		C[0][1] += d.X * d.Y;
		C[0][2] += d.X * d.Z;
		C[1][0] += d.Y * d.X;
		C[1][1] += d.Y * d.Y;
		C[1][2] += d.Y * d.Z;
		C[2][0] += d.Z * d.X;
		C[2][1] += d.Z * d.Y;
		C[2][2] += d.Z * d.Z;
	}
	return PowerIteratePCA(C);
}

static int PlaneTriangleIntersection(
	const FVector& V0, const FVector& V1, const FVector& V2,
	const FVector& PlaneOrigin, const FVector& PlaneNormal,
	TArray<FVector>& OutPoints)
{
	const float d0 = FVector::DotProduct(V0 - PlaneOrigin, PlaneNormal);
	const float d1 = FVector::DotProduct(V1 - PlaneOrigin, PlaneNormal);
	const float d2 = FVector::DotProduct(V2 - PlaneOrigin, PlaneNormal);

	const float eps = 1e-5f;
	const bool s0 = FMath::Abs(d0) < eps;
	const bool s1 = FMath::Abs(d1) < eps;
	const bool s2 = FMath::Abs(d2) < eps;

	int nIntersect = 0;

	auto CheckEdge = [&](const FVector& A, const FVector& B, float dA, float dB)
	{
		if (FMath::Sign(dA) != FMath::Sign(dB) && FMath::Abs(dA) > eps && FMath::Abs(dB) > eps)
		{
			const float t = dA / (dA - dB);
			OutPoints.Add(FMath::Lerp(A, B, t));
			nIntersect++;
		}
	};

	CheckEdge(V0, V1, d0, d1);
	CheckEdge(V1, V2, d1, d2);
	CheckEdge(V2, V0, d2, d0);

	if (s0 && !s1 && !s2)
	{
		OutPoints.Add(V0);
		nIntersect++;
	}
	if (s1 && !s0 && !s2)
	{
		OutPoints.Add(V1);
		nIntersect++;
	}
	if (s2 && !s0 && !s1)
	{
		OutPoints.Add(V2);
		nIntersect++;
	}

	return nIntersect;
}

bool UTunnelSectionCenterlineLibrary::ComputePlaneSectionCentroid(
	const TArray<FVector>& Pos,
	const TArray<FIntVector>& Tris,
	const FVector& Origin,
	const FVector& Normal,
	FVector& OutCentroid)
{
	TArray<FVector> IntersectionPoints;
	IntersectionPoints.Reserve(Tris.Num() * 2);

	for (const FIntVector& T : Tris)
	{
		PlaneTriangleIntersection(Pos[T.X], Pos[T.Y], Pos[T.Z], Origin, Normal, IntersectionPoints);
	}

	if (IntersectionPoints.Num() == 0)
	{
		return false;
	}

	FVector Sum(0, 0, 0);
	for (const FVector& P : IntersectionPoints)
	{
		Sum += P;
	}
	OutCentroid = Sum / float(IntersectionPoints.Num());
	return true;
}

TArray<FVector> UTunnelSectionCenterlineLibrary::TraceCentroids(
	const TArray<FVector>& Pos,
	const TArray<FIntVector>& Tris,
	const FBox& Bounds,
	FVector Origin,
	FVector Dir,
	float Step,
	int32 MaxSteps)
{
	TArray<FVector> Centers;
	Centers.Reserve(MaxSteps);

	FVector CurDir = Dir.GetSafeNormal();
	FVector CurOrigin = Origin;

	FVector FirstCenter;
	bool bFoundFirst = false;
	for (int warmup = 0; warmup < 32; ++warmup)
	{
		if (ComputePlaneSectionCentroid(Pos, Tris, CurOrigin, CurDir, FirstCenter))
		{
			bFoundFirst = true;
			break;
		}
		CurOrigin += CurDir * (Step * 0.3f);
	}

	if (!bFoundFirst)
	{
		UAutoParse::PrintLog_GameThread(FString::Printf(TEXT("[TraceCentroids] Could not find first valid section.")));
		return Centers;
	}

	Centers.Add(FirstCenter);

	int32 ConsecutiveNoHit = 0;
	const int32 MaxConsecutiveNoHit = 8;
	const float MaxDistFromPrev = Step * 3.0f;
	FVector LastValidCenter = FirstCenter;

	for (int32 i = 1; i <= MaxSteps; ++i)
	{
		CurOrigin += CurDir * Step;

		FVector C;
		bool bHit = ComputePlaneSectionCentroid(Pos, Tris, CurOrigin, CurDir, C);

		if (!bHit)
		{
			const float Tiny = Step * 0.2f;
			for (int k = -3; k <= 3; ++k)
			{
				if (k == 0)
				{
					continue;
				}
				const FVector testOrigin = CurOrigin + CurDir * (k * Tiny);
				if (ComputePlaneSectionCentroid(Pos, Tris, testOrigin, CurDir, C))
				{
					CurOrigin = testOrigin;
					bHit = true;
					break;
				}
			}
		}

		if (!bHit)
		{
			ConsecutiveNoHit++;

			if (ConsecutiveNoHit >= MaxConsecutiveNoHit)
			{
				bool bFoundBeyond = false;
				FVector ExtendOrigin = CurOrigin;
				for (int ext = 1; ext <= 8; ++ext)
				{
					ExtendOrigin += CurDir * Step;
					if (ComputePlaneSectionCentroid(Pos, Tris, ExtendOrigin, CurDir, C))
					{
						bFoundBeyond = true;
						CurOrigin = ExtendOrigin;
						bHit = true;
						ConsecutiveNoHit = 0;
						break;
					}
				}

				if (!bFoundBeyond)
				{
					break;
				}
			}

			if (!bHit)
			{
				continue;
			}
		}

		const float DistFromPrev = FVector::Dist(C, LastValidCenter);
		if (DistFromPrev > MaxDistFromPrev)
		{
			UAutoParse::PrintLog_GameThread(
				FString::Printf(TEXT("[TraceCentroids] Centroid jumped too far, stopping.")));
			break;
		}

		ConsecutiveNoHit = 0;
		LastValidCenter = C;

		if (Centers.Num() > 0)
		{
			FVector NewDir = (C - Centers.Last());
			if (NewDir.Normalize())
			{
				CurDir = (CurDir * 0.7f + NewDir * 0.3f).GetSafeNormal();
			}
		}

		Centers.Add(C);
	}

	return Centers;
}

bool UTunnelSectionCenterlineLibrary::ExtendToOpening(
	const TArray<FVector>& Pos,
	const TArray<FIntVector>& Tris,
	FVector StartCentroid,
	FVector Direction,
	float StepSize,
	int32 MaxSteps,
	TArray<FVector>& OutExtensionPoints)
{
	OutExtensionPoints.Reset();

	FVector CurOrigin = StartCentroid;
	FVector CurDir = Direction.GetSafeNormal();

	int32 ConsecutiveNoHit = 0;
	const int32 MaxNoHit = 5;
	int32 TotalNoHitCount = 0;
	const int32 MaxTotalNoHit = 15;

	for (int32 i = 0; i < MaxSteps; ++i)
	{
		CurOrigin += CurDir * StepSize;

		FVector C;
		bool bHit = ComputePlaneSectionCentroid(Pos, Tris, CurOrigin, CurDir, C);

		if (!bHit)
		{
			bool bFoundNearby = false;
			const float MicroStep = StepSize * 0.3f;

			for (int k = 1; k <= 5; ++k)
			{
				FVector ProbeOrigin = CurOrigin + CurDir * (k * MicroStep);
				if (ComputePlaneSectionCentroid(Pos, Tris, ProbeOrigin, CurDir, C))
				{
					CurOrigin = ProbeOrigin;
					bHit = true;
					bFoundNearby = true;
					UAutoParse::PrintLog_GameThread(FString::Printf(
						TEXT("[ExtendToOpening] Found section with micro-probe at offset %.1f"),
						k * MicroStep));
					break;
				}
			}

			if (!bFoundNearby)
			{
				ConsecutiveNoHit++;
				TotalNoHitCount++;

				if (ConsecutiveNoHit >= MaxNoHit)
				{
					UAutoParse::PrintLog_GameThread(FString::Printf(
						TEXT("[ExtendToOpening] Reached opening after %d extension points (consecutive no-hit: %d)"),
						OutExtensionPoints.Num(), ConsecutiveNoHit));
					return OutExtensionPoints.Num() > 0;
				}

				if (TotalNoHitCount >= MaxTotalNoHit)
				{
					UAutoParse::PrintLog_GameThread(FString::Printf(
						TEXT("[ExtendToOpening] Exceeded total no-hit limit (%d), stopping."),
						MaxTotalNoHit));
					return OutExtensionPoints.Num() > 0;
				}

				continue;
			}
		}

		if (bHit)
		{
			ConsecutiveNoHit = 0;
			OutExtensionPoints.Add(C);

			if (OutExtensionPoints.Num() >= 2)
			{
				FVector NewDir = (C - OutExtensionPoints[OutExtensionPoints.Num() - 2]);
				if (NewDir.Normalize())
				{
					CurDir = (CurDir * 0.85f + NewDir * 0.15f).GetSafeNormal();
				}
			}
		}
	}

	if (OutExtensionPoints.Num() > 0)
	{
		UAutoParse::PrintLog_GameThread(FString::Printf(
			TEXT("[ExtendToOpening] Reached max steps (%d), extended %d points."),
			MaxSteps, OutExtensionPoints.Num()));
	}

	return OutExtensionPoints.Num() > 0;
}

void UTunnelSectionCenterlineLibrary::ResampleUniform(const TArray<FVector>& In, float Spacing, TArray<FVector>& Out)
{
	Out.Reset();
	if (In.Num() < 2)
	{
		Out = In;
		return;
	}

	TArray<double> cum;
	cum.SetNum(In.Num());
	cum[0] = 0.0;
	for (int i = 1; i < In.Num(); ++i)
	{
		cum[i] = cum[i - 1] + FVector::Dist(In[i - 1], In[i]);
	}
	const double total = cum.Last();
	if (total <= KINDA_SMALL_NUMBER)
	{
		Out = In;
		return;
	}

	const int nSamples = FMath::Max(2, int(FMath::CeilToInt(total / Spacing)));
	for (int i = 0; i <= nSamples; ++i)
	{
		const double want = (double(i) / double(nSamples)) * total;
		int idx = 0;
		while (idx + 1 < cum.Num() && cum[idx + 1] < want)
		{
			idx++;
		}
		if (idx + 1 >= cum.Num())
		{
			Out.Add(In.Last());
			continue;
		}
		const double seg = cum[idx + 1] - cum[idx];
		const double a = (seg > 0) ? ((want - cum[idx]) / seg) : 0.0;
		Out.Add(FMath::Lerp(In[idx], In[idx + 1], float(a)));
	}
}

USplineComponent* UTunnelSectionCenterlineLibrary::CreateOrFindSpline(AActor* Owner, FName Name, bool bReplace)
{
	// if (!Owner)
	// {
	// 	return nullptr;
	// }
	// USplineComponent* Found = nullptr;
	// TArray<USplineComponent*> Comps;
	// Owner->GetComponents(Comps);
	// for (USplineComponent* C : Comps)
	// {
	// 	if (C && C->GetFName() == Name)
	// 	{
	// 		Found = C;
	// 		break;
	// 	}
	// }
	// if (Found && bReplace)
	// {
	// 	Found->ClearSplinePoints(false);
	// 	return Found;
	// }
	// if (!Found)
	// {
	// 	USplineComponent* NewC = NewObject<USplineComponent>(Owner, Name);
	// 	NewC->SetupAttachment(Owner->GetRootComponent());
	// 	NewC->RegisterComponent();
	// 	NewC->SetMobility(EComponentMobility::Movable);
	// 	return NewC;
	// }
	// return Found;
	if (!Owner) return nullptr;

	TArray<USplineComponent*> Comps;
	Owner->GetComponents(Comps);
	for (USplineComponent* C : Comps)
	{
		if (C && C->GetFName() == Name)
		{
			if (bReplace)
			{
				C->ClearSplinePoints(false);
			}
			return C;
		}
	}

	USplineComponent* NewC = NewObject<USplineComponent>(Owner, Name, RF_Transactional);
	if (!NewC) return nullptr;

#if WITH_EDITOR
	NewC->CreationMethod = EComponentCreationMethod::Instance;
#endif

	Owner->AddInstanceComponent(NewC);

	USceneComponent* ParentAttach = Owner->GetRootComponent();
	if (ParentAttach)
	{
		NewC->AttachToComponent(ParentAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	NewC->RegisterComponent();
	NewC->SetMobility(EComponentMobility::Movable);

	return NewC;
}

void UTunnelSectionCenterlineLibrary::WriteWorldToSpline(USplineComponent* Spline, const TArray<FVector>& Pts)
{
	if (!Spline)
	{
		return;
	}
	Spline->ClearSplinePoints(false);
	for (const FVector& P : Pts)
	{
		Spline->AddSplinePoint(P, ESplineCoordinateSpace::World, false);
	}
	Spline->SetClosedLoop(false, false);
	Spline->UpdateSpline();
}

void UTunnelSectionCenterlineLibrary::DrawDebugPolyline(UWorld* World, const TArray<FVector>& Pts, FColor Color,
                                                        float Time)
{
	if (!World || Pts.Num() < 2)
	{
		return;
	}
	for (int i = 0; i < Pts.Num() - 1; ++i)
	{
		DrawDebugLine(World, Pts[i], Pts[i + 1], Color, false, Time, 0, 2.f);
	}
	for (const FVector& P : Pts)
	{
		DrawDebugSphere(World, P, 8.f, 16, Color, false, Time);
	}
}

int32 UTunnelSectionCenterlineLibrary::ComputeSectionIntersectionCount(const TArray<FVector>& Pos,
                                                                       const TArray<FIntVector>& Tris,
                                                                       const FVector& Origin, const FVector& Normal)
{
	TArray<FVector> IntersectionPoints;
	IntersectionPoints.Reserve(Tris.Num() * 2);

	for (const FIntVector& T : Tris)
	{
		PlaneTriangleIntersection(Pos[T.X], Pos[T.Y], Pos[T.Z], Origin, Normal, IntersectionPoints);
	}

	return IntersectionPoints.Num();
}

int32 UTunnelSectionCenterlineLibrary::ExtendToOpeningAggressive(const TArray<FVector>& Pos,
                                                                 const TArray<FIntVector>& Tris, const FBox& Bounds,
                                                                 FVector StartCentroid, FVector Direction,
                                                                 float StepSize,
                                                                 int32 MaxSteps, TArray<FVector>& OutExtensionPoints)
{
	OutExtensionPoints.Reset();

	FVector CurOrigin = StartCentroid;
	FVector CurDir = Direction.GetSafeNormal();

	const int32 InitialIntersectionCount = ComputeSectionIntersectionCount(Pos, Tris, CurOrigin, CurDir);
	const int32 MinIntersectionThreshold = FMath::Max(4, InitialIntersectionCount / 5);

	UAutoParse::PrintLog_GameThread(FString::Printf(
		TEXT("[ExtendAggressive] Initial intersection count: %d, threshold: %d"),
		InitialIntersectionCount, MinIntersectionThreshold));

	int32 StepsWithoutValidCentroid = 0;
	const int32 MaxStepsWithoutValid = 12;

	const FVector BoundsCenter = Bounds.GetCenter();
	const float BoundsMaxExtent = Bounds.GetExtent().Size();

	for (int32 i = 0; i < MaxSteps; ++i)
	{
		CurOrigin += CurDir * StepSize;

		const float DistFromBoundsCenter = FVector::Dist(CurOrigin, BoundsCenter);
		if (DistFromBoundsCenter > BoundsMaxExtent * 1.5f)
		{
			UAutoParse::PrintLog_GameThread(FString::Printf(
				TEXT("[ExtendAggressive] Exceeded bounds (dist %.1f > %.1f), stopping at %d points."),
				DistFromBoundsCenter, BoundsMaxExtent * 1.5f, OutExtensionPoints.Num()));
			break;
		}

		const int32 CurIntersectionCount = ComputeSectionIntersectionCount(Pos, Tris, CurOrigin, CurDir);

		FVector C;
		bool bHasValidCentroid = false;
		if (CurIntersectionCount > 0)
		{
			bHasValidCentroid = ComputePlaneSectionCentroid(Pos, Tris, CurOrigin, CurDir, C);
		}

		if (bHasValidCentroid)
		{
			StepsWithoutValidCentroid = 0;
			OutExtensionPoints.Add(C);

			if (OutExtensionPoints.Num() >= 2)
			{
				FVector NewDir = (C - OutExtensionPoints[OutExtensionPoints.Num() - 2]);
				if (NewDir.Normalize())
				{
					CurDir = (CurDir * 0.9f + NewDir * 0.1f).GetSafeNormal();
				}
			}

			if (CurIntersectionCount < MinIntersectionThreshold && OutExtensionPoints.Num() >= 3)
			{
				UAutoParse::PrintLog_GameThread(FString::Printf(TEXT(
					"[ExtendAggressive] Section area decayed (count %d < %d), likely near opening. Extended %d points."
				),
				                                                CurIntersectionCount, MinIntersectionThreshold,
				                                                OutExtensionPoints.Num()));
				int32 ConfirmSteps = 0;
				FVector ConfirmOrigin = CurOrigin;
				for (int k = 1; k <= 5; ++k)
				{
					ConfirmOrigin += CurDir * StepSize;
					FVector ConfirmC;
					if (ComputePlaneSectionCentroid(Pos, Tris, ConfirmOrigin, CurDir, ConfirmC))
					{
						OutExtensionPoints.Add(ConfirmC);
						ConfirmSteps++;
					}
					else
					{
						break;
					}
				}
				UAutoParse::PrintLog_GameThread(
					FString::Printf(TEXT("[ExtendAggressive] Confirmed with %d additional steps."), ConfirmSteps));
				break;
			}
		}
		else
		{
			StepsWithoutValidCentroid++;

			if (CurIntersectionCount > 0 && CurIntersectionCount < MinIntersectionThreshold)
			{
				UAutoParse::PrintLog_GameThread(FString::Printf(
					TEXT("[ExtendAggressive] Step %d: no centroid but %d intersections (edge region)"),
					i, CurIntersectionCount));
				StepsWithoutValidCentroid = FMath::Max(0, StepsWithoutValidCentroid - 1);
			}

			if (StepsWithoutValidCentroid >= MaxStepsWithoutValid)
			{
				UAutoParse::PrintLog_GameThread(FString::Printf(
					TEXT("[ExtendAggressive] %d consecutive steps without valid centroid, stopping at %d points."),
					MaxStepsWithoutValid, OutExtensionPoints.Num()));
				break;
			}
		}
	}

	UAutoParse::PrintLog_GameThread(
		FString::Printf(TEXT("[ExtendAggressive] Final extension: %d points."), OutExtensionPoints.Num()));
	return OutExtensionPoints.Num();
}