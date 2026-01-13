#include "MainUI/MenuEntryWidgetBase.h"

#include "TagEvent/ZHJZTagEventBusSubsystem.h"

void UMenuEntryWidgetBase::ApplyRowData(const FMenuEntryConfig& InRow)
{
	RowData = InRow;
	BP_OnRowDataApplied();
}

void UMenuEntryWidgetBase::HandleClicked()
{
	if (UZHJZTagEventBusSubsystem* Bus = GetGameInstance()->GetSubsystem<UZHJZTagEventBusSubsystem>())
	{
		Bus->BroadcastTagEvent(RowData.LabelTag, RowData.JsonRaw);
	}
}

FGameplayTag UMenuEntryWidgetBase::GetLabelTag() const
{
	return RowData.LabelTag;
}
