#include "AwsaPipeSubsystem.h"

using namespace UE::Tasks;

UAwsaPipeSubsystem::UAwsaPipeSubsystem()
{
}

bool UAwsaPipeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void UAwsaPipeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Pipe = MakeUnique<FPipe>(UE_SOURCE_LOCATION);
}

void UAwsaPipeSubsystem::Deinitialize()
{
	Pipe.Reset();
	Super::Deinitialize();
}
