#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AwsaPipeSubsystem.generated.h"

using namespace UE::Tasks;

UCLASS()
class AWSAPIPE_API UAwsaPipeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAwsaPipeSubsystem();
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	template <typename Functor, typename PrereqType = TArray<FTask>>
	FTask AddTask(
		Functor&& TaskFunc,
		PrereqType&& Prerequisites = {},
		ETaskPriority Priority = ETaskPriority::Normal)
	{
		ensure(Pipe.IsValid());

		TArray<FTask> PrereqArray;
		ConvertToArray(PrereqArray, Forward<PrereqType>(Prerequisites));

		if (PrereqArray.Num() == 0)
		{
			return Pipe->Launch(UE_SOURCE_LOCATION, Forward<Functor>(TaskFunc), Priority);
		}
		else
		{
			return Pipe->Launch(UE_SOURCE_LOCATION, Forward<Functor>(TaskFunc), PrereqArray, Priority);
		}
	}

private:
	void ConvertToArray(TArray<FTask>& OutArray, const FTask& Single)
	{
		if (Single.IsValid())
		{
			OutArray.Add(Single);
		}
	}

	void ConvertToArray(TArray<FTask>& OutArray, const TArray<FTask>& Arr)
	{
		OutArray = Arr;
	}

	TUniquePtr<FPipe> Pipe;
};
