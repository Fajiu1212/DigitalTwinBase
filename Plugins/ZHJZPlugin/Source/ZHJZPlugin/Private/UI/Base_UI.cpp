// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Base_UI.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Blueprint/WidgetTree.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"


void UBase_UI::NativeConstruct()
{
	Super::NativeConstruct();

	AddClickedSound();

	//AddCheckBoxClickedSound();
}

void UBase_UI::AddClickedSound()
{
	if (WidgetTree)
	{
		//UI所有控件
		TArray<UWidget*> Widgets;

		WidgetTree->GetAllWidgets(Widgets);

		for (UWidget* Widget : Widgets)
		{
			if (Widget)
			{
				//Button
				bool IsButton= Widget->IsA(UButton::StaticClass());
				if (IsButton)
				{
					UButton* Btn = Cast<UButton>(Widget);
					if (Btn)
					{
						FScriptDelegate Del;
						Del.BindUFunction(this, TEXT("PlayBtnOnClickSound"));
						Btn->OnClicked.Add(Del);
						continue;
					}
				}
				
				//CheckBox
				bool IsCheckBox = Widget->IsA(UCheckBox::StaticClass());
				if (IsCheckBox)
				{
					UCheckBox* CheckBox = Cast<UCheckBox>(Widget);
					if (CheckBox)
					{
						FScriptDelegate Del2;
						Del2.BindUFunction(this, TEXT("PlayCheckBoxOnClickSound"));
						CheckBox->OnCheckStateChanged.Add(Del2);
					}
				}
			}
		}
	}
}



void UBase_UI::PlayBtnOnClickSound()
{
	if (IsPlaySound)
	{
		if (Sound)
		{
			Sound->bLooping = false;
			UGameplayStatics::PlaySound2D(GetWorld(), Sound, 1.f);
		}

	}
}


void UBase_UI::PlayCheckBoxOnClickSound()
{
	if (IsPlaySound)
	{
		if (Sound)
		{
			Sound->bLooping = false;
			UGameplayStatics::PlaySound2D(GetWorld(), Sound, 1.f);
		}
	}
}


