#pragma once

#include "CoreMinimal.h"
#include "AwsaDelegate.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FTestDelegate,FString,str);

USTRUCT(BlueprintType)
struct FTestStr
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsTest;
	FTestStr() = default;
};