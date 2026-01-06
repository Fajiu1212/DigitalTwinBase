#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseManager.generated.h"

UCLASS(Blueprintable)
class ZHJZPLUGIN_API UBaseManager : public UObject
{
	GENERATED_BODY()
	
public:
	class UWorld* GetWorld() const;

	class ULevel* GetLevel() const;
};
