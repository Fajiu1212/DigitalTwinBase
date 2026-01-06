// Fill out your copyright notice in the Description page of Project Settings.


#include "Func/BIM_Func.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "Runtime/Engine/Public/EngineUtils.h"

void UBIM_Func::BIM_SetHiddenbyTag(const UObject* WorldContextObject, FName Tag, bool bNewHidden)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			AStaticMeshActor* Mesh = *ActorItr;
			if (Mesh && IsValid(Mesh) && Mesh->ActorHasTag(Tag))
			{
				if (Mesh->IsHidden() != bNewHidden)
				{
					Mesh->SetHidden(bNewHidden);
					Mesh->MarkComponentsRenderStateDirty();
				}
			}
		}
	}
}
void UBIM_Func::BIM_SetHiddenbyTags(const UObject* WorldContextObject, TArray<FName> Tags, bool bNewHidden)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (int num1 = 0; num1 < Tags.Num(); num1++)
		{
			for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
			{
				AStaticMeshActor* Mesh = *ActorItr;
				if (Mesh && !IsValid(Mesh) && Mesh->ActorHasTag(Tags[num1]))
				{
					if (Mesh->IsHidden() != bNewHidden)
					{
						Mesh->SetHidden(bNewHidden);
						Mesh->MarkComponentsRenderStateDirty();
					}
				}
			}
		}
	}
}


void UBIM_Func::BIM_SetClassHiddenInGame(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, bool bNewHidden)
{
	if (ActorClass)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
			{
				AActor* Actor = *It;
				Actor->SetActorHiddenInGame(bNewHidden);
			}
		}
	}

}
void UBIM_Func::BIM_SetAllClassesHiddenInGame(const UObject* WorldContextObject, TArray<TSubclassOf<AActor>> ActorClasses, bool bNewHidden)
{

	TArray<AActor*> OutActors;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{

		for (int num1 = 0; num1 < ActorClasses.Num(); num1++)
		{
			OutActors.Reset();
			if (ActorClasses[num1])
			{
				const TSubclassOf<AActor>& ActorClass1 = ActorClasses[num1];
				UGameplayStatics::GetAllActorsOfClass(WorldContextObject, ActorClass1, OutActors);
				for (int i = 0; i < OutActors.Num(); i++)
				{
					if (OutActors[i]->IsHidden() != bNewHidden)
					{
						OutActors[i]->SetHidden(bNewHidden);
						OutActors[i]->MarkComponentsRenderStateDirty();
					}
				}
			}
		}
	}
}


void UBIM_Func::BIM_SetHiddenAndCollisionbyTag(const UObject* WorldContextObject, FName Tag, FName ProfileName, bool bNewHidden)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			AStaticMeshActor* Mesh = *ActorItr;
			if (Mesh && IsValid(Mesh) && Mesh->ActorHasTag(Tag))
			{
				if (Mesh->IsHidden() != bNewHidden)
				{
					Mesh->SetHidden(bNewHidden);
					Mesh->MarkComponentsRenderStateDirty();
					if (bNewHidden)
					{
						Mesh->GetStaticMeshComponent()->SetCollisionProfileName(ProfileName, true);
					}
					else
					{
						Mesh->GetStaticMeshComponent()->SetCollisionProfileName(ProfileName, true);
					}
				}
			}
		}
	}
}
void UBIM_Func::BIM_SetHiddenAndCollisionbyTags(const UObject* WorldContextObject, TArray<FName> Tags, FName ProfileName, bool bNewHidden)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (int num1 = 0; num1 < Tags.Num(); num1++)
		{
			for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
			{
				AStaticMeshActor* Mesh = *ActorItr;
				if (Mesh && IsValid(Mesh) && Mesh->ActorHasTag(Tags[num1]))
				{
					if (Mesh->IsHidden() != bNewHidden)
					{
						Mesh->SetHidden(bNewHidden);
						Mesh->MarkComponentsRenderStateDirty();
						if (bNewHidden)
						{
							Mesh->GetStaticMeshComponent()->SetCollisionProfileName(ProfileName, true);
						}
						else
						{
							Mesh->GetStaticMeshComponent()->SetCollisionProfileName(ProfileName, true);
						}
					}

				}
			}
		}
	}
}



void UBIM_Func::BIM_SetMaterialByTag(const UObject* WorldContextObject, FName Tag, int32 ElementIndex, UMaterialInterface* Material)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)) {
		for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			AStaticMeshActor* Mesh = *ActorItr;
			if (Mesh && IsValid(Mesh) && Mesh->ActorHasTag(Tag))
			{
				if (Mesh->GetStaticMeshComponent()->GetMaterial(0) != Material)
					Mesh->GetStaticMeshComponent()->SetMaterial(ElementIndex, Material);

			}
		}
	}
}

void UBIM_Func::BIM_SetMaterialByTags(const UObject* WorldContextObject, TArray<FName> Tags, int32 ElementIndex, UMaterialInterface* Material)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		for (int num1 = 0; num1 < Tags.Num(); num1++)
		{
			for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
			{
				AStaticMeshActor* Mesh = *ActorItr;
				if (Mesh && IsValid(Mesh) && Mesh->ActorHasTag(Tags[num1]))
				{
					if (Mesh->GetStaticMeshComponent()->GetMaterial(0) != Material)
						Mesh->GetStaticMeshComponent()->SetMaterial(ElementIndex, Material);
				}
			}
		}
	}
}

