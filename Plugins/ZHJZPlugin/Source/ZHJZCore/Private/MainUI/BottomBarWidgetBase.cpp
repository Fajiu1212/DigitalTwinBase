#include "MainUI/BottomBarWidgetBase.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "MainUI/BottomBarButtonWidgetBase.h"

static UWorld* ResolveWorldForWidget(UUserWidget* Widget, bool bIsDesignTime)
{
	if (!Widget)
	{
		return nullptr;
	}

	if (UWorld* World = Widget->GetWorld())
	{
		return World;
	}

	if (UWorld* OuterWorld = Widget->GetTypedOuter<UWorld>())
	{
		return OuterWorld;
	}

	if (UObject* Outer = Widget->GetOuter())
	{
		return Outer->GetWorld();
	}

	return nullptr;
}

void UBottomBarWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	RebuildFromConfig(IsDesignTime(), DefaultBottomBarStyle);
}

void UBottomBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	RebuildFromConfig(IsDesignTime(), DefaultBottomBarStyle);
}

void UBottomBarWidgetBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	RebuildFromConfig(IsDesignTime(), DefaultBottomBarStyle);
}

void UBottomBarWidgetBase::RebuildFromConfig(bool bIsDesignTime, FGameplayTag TargetStyle)
{
	ClearBoxes();

	if (!Config || !Config->BottomBarConfig.Num())
	{
		return;
	} 

	if (!BottomBarBox)
	{
		return;
	}

	// Target Style
	const FBottomBarConfigStruct* TargetStyleConfigStruct = nullptr;
	
	// TO DO : Rebuild by style tag
	if (TargetStyle.IsValid())
	{
		for (const TPair<FGameplayTag, FBottomBarConfigStruct>& Pair : Config->BottomBarConfig)
		{
			if (Pair.Key.IsValid() && Pair.Key == TargetStyle)
			{
				TargetStyleConfigStruct = &Pair.Value;
				break;
			}
		}
	}

	if (!TargetStyleConfigStruct)
	{
		return;
	}
	
	
	UWorld* World = ResolveWorldForWidget(this, bIsDesignTime);
	if (!World)
	{
		return;
	}

	// Create && Add
	auto CreateAndAdd = [&](const FBottomBarButtonConfig& Cfg)
	{
		if (!BottomBarBox)
		{
			return;
		}

		if (Cfg.ButtonWidgetClass.IsNull())
		{
			return;
		}

		UClass* ButtonClass = Cfg.ButtonWidgetClass.LoadSynchronous();
		if (!ButtonClass)
		{
			return;
		}

		UBottomBarButtonWidgetBase* Button = CreateWidget<UBottomBarButtonWidgetBase>(World, ButtonClass);
		if (!Button)
		{
			return;
		}
		Button->ApplyRowData(Cfg);

		AddButtonToBox(Button, Cfg);
	};

	for (const FBottomBarButtonConfig& Cfg : TargetStyleConfigStruct->BottomBarButtons)
	{
		CreateAndAdd(Cfg);
	}
}

void UBottomBarWidgetBase::ClearBoxes()
{
	if (BottomBarBox)
	{
		BottomBarBox->ClearChildren();
	}
}

void UBottomBarWidgetBase::AddButtonToBox(UBottomBarButtonWidgetBase* ButtonWidget,
	const struct FBottomBarButtonConfig& Cfg)
{
	if (!ButtonWidget)
	{
		return;
	}
	
	if (!BottomBarBox)
	{
		return;
	}
	
	UHorizontalBoxSlot* MySlot = BottomBarBox->AddChildToHorizontalBox(ButtonWidget);
	if (!MySlot)
	{
		return;
	}

	// slot 样式
	MySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	MySlot->SetVerticalAlignment(VAlign_Fill);
	
	const float BasePadX = 4.0f;
	const float BasePadY = 4.0f;
	
	FMargin Pad(BasePadX, BasePadY);

	MySlot->SetPadding(Pad);
}

void UBottomBarWidgetBase::SortGroup(TArray<const struct FBottomBarButtonConfig*>& InOut)
{
	InOut.Sort([](const FBottomBarButtonConfig& A, const FBottomBarButtonConfig& B)
	{
		if (A.Index != B.Index)
		{
			return A.Index < B.Index;
		}
		return A.ButtonName.ToString() < B.ButtonName.ToString();
	});
}