#include "DigitalTwinBase/Public/AwsaDelegateTestActor.h"


AAwsaDelegateTestActor::AAwsaDelegateTestActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAwsaDelegateTestActor::BeginPlay()
{
	Super::BeginPlay();
	if (TestDelegate.IsBound())
	{
		TestDelegate.Execute(TEXT("On delegate initialized is called"));
	}
}

void AAwsaDelegateTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
