#include "MainUI/BottomBarButtonWidgetBase.h"

#include "Components/VerticalBoxSlot.h"
#include "TagEvent/ZHJZTagEventBusSubsystem.h"

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

void UBottomBarButtonWidgetBase::ApplyRowData(const FBottomBarButtonConfig& InRow)
{
	RowData = InRow;
	AddMenuEntry();
	BP_OnRowDataApplied();
}


void UBottomBarButtonWidgetBase::HandleClicked()
{
	if (UZHJZTagEventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UZHJZTagEventBusSubsystem>())
	{
		Bus->BroadcastTagEvent(RowData.ButtonTag, RowData.JsonRaw);
	}
}

FGameplayTag UBottomBarButtonWidgetBase::GetButtonTag() const
{
	return RowData.ButtonTag;
}

void UBottomBarButtonWidgetBase::AddButtonToBox_Implementation(UMenuEntryWidgetBase* ButtonWidget)
{
	if (!ButtonWidget)
	{
		return;
	}
	
	UVerticalBoxSlot* MySlot = VerticalBox_MenuEntryBox.Get()->AddChildToVerticalBox(ButtonWidget);
	
	if (!MySlot) return;
	MySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	MySlot->SetHorizontalAlignment(HAlign_Left);
	MySlot->SetVerticalAlignment(VAlign_Fill);
}

void UBottomBarButtonWidgetBase::AddMenuEntry_Implementation()
{
	UWorld* World = ResolveWorldForWidget(this, IsDesignTime());
	
	// Create && Add
	auto CreateAndAdd = [&](FMenuEntryConfig Cfg)
	{
		if (Cfg.MenuEntryWidgetClass.IsNull())
		{
			return;
		}

		
		UClass* MenuEntryClass = Cfg.MenuEntryWidgetClass.LoadSynchronous();
		if (!MenuEntryClass)
		{
			return;
		}

		UMenuEntryWidgetBase* Button = CreateWidget<UMenuEntryWidgetBase>(World, MenuEntryClass);
		if (!Button)
		{
			return;
		}

		Button->ApplyRowData(Cfg);

		AddButtonToBox(Button);
	};
	
	for (const FMenuEntryConfig Cfg : RowData.BelongMenuEntryButtonConfigs)
	{
		CreateAndAdd(Cfg);
	}
}


