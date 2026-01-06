/**
* Desc: 枚举定义
* Author:
* Date:
*/

#pragma once

#include "CoreMinimal.h"

//FString->Enum 或者 Enum->FString
//只能对使用UENUM()宏的枚举使用
namespace EnumHelper
{
	template <typename TEnum>
	static FString GetStringFromTEnum(const FString& EnumName, const TEnum& Value)
	{
		//Old Api
		//const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		const UEnum* EnumPtr = FindObject<UEnum>(nullptr, *EnumName, true);
		if (!EnumPtr)
		{
			return FString(TEXT("InValid"));
		}
		//Old Api
		//return EnumPtr->GetNameStringByIndex((int32)Value);
		return EnumPtr->GetNameStringByIndex(static_cast<int32>(Value));
	}

	template <typename TEnum>
	static TEnum GetEnumValueFromString(const FString& EnumName, const FString& Value)
	{
		//Old Api
		//const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		const UEnum* EnumPtr = FindObject<UEnum>(nullptr, *EnumName, true);
		if (!EnumPtr)
		{
			return TEnum(-1);
		}
		//Old Api
		//return (TEnum)EnumPtr->GetIndexByName(FName(*Value));
		return static_cast<TEnum>(EnumPtr->GetIndexByName(FName(*Value)));
	}
}

//测试
UENUM(BlueprintType)
enum class ETestType : uint8
{
	Test_1 = 0,
	Test_2,
	Test_3,
	Test_4,
};

//项目类型
UENUM(BlueprintType)
enum class EProjType : uint8
{
	ZHJZ = 0,
	EQ,
	SQ,
	MR,
	LC,
};
