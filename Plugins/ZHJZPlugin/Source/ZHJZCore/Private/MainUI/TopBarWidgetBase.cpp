#include "MainUI/TopBarWidgetBase.h"

#include "Blueprint/WidgetTree.h"
#include "MainUI/TopBarButtonWidgetBase.h"
#include "MainUI/TopBarConfigAsset.h"


#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"


void UTopBarWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	RebuildFromConfig(IsDesignTime());
}

void UTopBarWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	RebuildFromConfig(IsDesignTime());
}

void UTopBarWidgetBase::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	RebuildFromConfig(IsDesignTime());
}

void UTopBarWidgetBase::ClearBoxes()
{
	if (LeftBox)
	{
		LeftBox->ClearChildren();
	}
	if (RightBox)
	{
		RightBox->ClearChildren();
	}
}

void UTopBarWidgetBase::SortGroup(TArray<const FTopBarButtonConfig*>& InOut)
{
	InOut.Sort([](const FTopBarButtonConfig& A, const FTopBarButtonConfig& B)
	{
		if (A.Index != B.Index)
		{
			return A.Index < B.Index;
		}
		return A.ButtonName.ToString() < B.ButtonName.ToString();
	});
}

void UTopBarWidgetBase::AddButtonToBox(
	UHorizontalBox* TargetBox,
	UTopBarButtonWidgetBase* ButtonWidget,
	const FTopBarButtonConfig& Cfg)
{
	if (!TargetBox || !ButtonWidget)
	{
		return;
	}

	UHorizontalBoxSlot* MySlot = TargetBox->AddChildToHorizontalBox(ButtonWidget);
	if (!MySlot)
	{
		return;
	}

	// slot 样式
	MySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	MySlot->SetVerticalAlignment(VAlign_Fill);

	const float BasePadX = 2.0f;
	const float BasePadY = 0.0f;
	const float Off = FMath::Max(0.f, Cfg.Offset);

	FMargin Pad(BasePadX, BasePadY);

	if (Cfg.Side == ETopBarSide::Left)
	{
		Pad.Left = BasePadX + Off;
		Pad.Right = BasePadX;
	}
	else
	{
		Pad.Left = BasePadX;
		Pad.Right = BasePadX + Off;
	}

	MySlot->SetPadding(Pad);
}

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

void UTopBarWidgetBase::RebuildFromConfig(bool bIsDesignTime)
{
	ClearBoxes();

	if (!Config)
	{
		return;
	}

	if (!LeftBox || !RightBox)
	{
		return;
	}

	TArray<const FTopBarButtonConfig*> Left;
	TArray<const FTopBarButtonConfig*> Right;

	for (const FTopBarButtonConfig& It : Config->TopButtons)
	{
		(It.Side == ETopBarSide::Left ? Left : Right).Add(&It);
	}

	SortGroup(Left);
	SortGroup(Right);

	UWorld* World = ResolveWorldForWidget(this, bIsDesignTime);
	if (!World)
	{
		return;
	}

	auto CreateAndAdd = [&](const FTopBarButtonConfig* Cfg, UHorizontalBox* TargetBox)
	{
		if (!Cfg || !TargetBox)
		{
			return;
		}

		if (Cfg->ButtonWidgetClass.IsNull())
		{
			return;
		}

		UClass* ButtonClass = Cfg->ButtonWidgetClass.LoadSynchronous();
		if (!ButtonClass)
		{
			return;
		}

		UTopBarButtonWidgetBase* Button = CreateWidget<UTopBarButtonWidgetBase>(World, ButtonClass);
		if (!Button)
		{
			return;
		}

		Button->ApplyRowData(*Cfg);

		AddButtonToBox(TargetBox, Button, *Cfg);
	};

	for (const FTopBarButtonConfig* Cfg : Left)
	{
		CreateAndAdd(Cfg, LeftBox);
	}
	for (const FTopBarButtonConfig* Cfg : Right)
	{
		CreateAndAdd(Cfg, RightBox);
	}
}
