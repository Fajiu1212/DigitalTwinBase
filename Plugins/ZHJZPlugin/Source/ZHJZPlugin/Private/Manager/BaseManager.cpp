// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/BaseManager.h"

ULevel* UBaseManager::GetLevel() const
{
	return GetTypedOuter<ULevel>();
}

UWorld* UBaseManager::GetWorld() const
{
	if (!HasAnyFlags(RF_ClassDefaultObject) && ensureMsgf(GetOuter(),
		TEXT("Actor: %s has a null OuterPrivate in AActor::GetWorld()"), *GetFullName())
		&& !GetOuter()->HasAnyFlags(RF_BeginDestroyed) && !GetOuter()->IsUnreachable())
	{
		if (ULevel* Level = GetLevel())
		{
			return Level->OwningWorld;
		}
	}
	return nullptr;
}