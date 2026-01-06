// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PositionDisplayUI.h"
#include "Components/TextBlock.h"

void UPositionDisplayUI::SetPositionText(const FVector& Position)
{
	if (PositionText)
	{
		FString PositionString = FString::Printf(TEXT("X: %.2f\nY: %.2f\nZ: %.2f"),
			Position.X, Position.Y, Position.Z);
		PositionText->SetText(FText::FromString(PositionString));
	}
}
