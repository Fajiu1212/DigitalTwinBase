#include "MainUI/BottomBarWidgetBase.h"

#include "ZHJZWidgetWorldUtils.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "MainUI/BottomBarButtonWidgetBase.h"
#include "TagEvent/ZHJZTagEventBusSubsystem.h"

void UBottomBarWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	RebuildFromConfig(DefaultBottomBarStyle);
}

void UBottomBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	RebuildFromConfig(DefaultBottomBarStyle);
	if (UZHJZTagEventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UZHJZTagEventBusSubsystem>())
	{
		Bus->RebuildBottomBarStyle.Clear();
		Bus->RebuildBottomBarStyle.BindDynamic(this, &UBottomBarWidgetBase::RebuildFromConfig);
	}
}

void UBottomBarWidgetBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	RebuildFromConfig(DefaultBottomBarStyle);
}

void UBottomBarWidgetBase::RebuildFromConfig(FGameplayTag TargetStyle)
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
	
	
	UWorld* World = ZHJZWidgetWorldUtils::ResolveWorld(this);
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