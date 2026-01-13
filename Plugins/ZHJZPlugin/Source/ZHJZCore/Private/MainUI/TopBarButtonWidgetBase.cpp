#include "MainUI/TopBarButtonWidgetBase.h"

#include "TagEvent/ZHJZTagEventBusSubsystem.h"


void UTopBarButtonWidgetBase::ApplyRowData(const FTopBarButtonConfig& InRow)
{
	RowData = InRow;
	BP_OnRowDataApplied();
}

void UTopBarButtonWidgetBase::HandleClicked()
{
	if (UZHJZTagEventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UZHJZTagEventBusSubsystem>())
	{
		Bus->BroadcastTagEvent(RowData.ButtonTag, RowData.JsonRaw);
	}
}

FGameplayTag UTopBarButtonWidgetBase::GetButtonTag() const
{
	return RowData.ButtonTag;
}
