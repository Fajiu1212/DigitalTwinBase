#include "UI/BeamWidgetBase.h"

void UBeamWidgetBase::SetBeamStruct_Implementation(const FBeamStruct& TargetBeamStruct)
{
	BeamStruct = TargetBeamStruct;
}

void UBeamWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBeamWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UBeamWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}
