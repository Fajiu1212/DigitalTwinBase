// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BIM_Func.generated.h"

/**
 * 
 */
UCLASS()
class ZHJZPLUGIN_API UBIM_Func : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		/*显隐类函数，有专门针对BIM模型的tag显隐，还有控制Actor与复数Acotr的显隐，不过并不能同时控制模型的碰撞消失或生成
	* 1:根据tag来控制staticmeshactor的显隐
	* 2:根据tag数组来控制拥有不同tag的staticmeshactor的显隐
	*
	* 3:根据actor类型来控制其显隐
	* 4:根据actor数组来控制其成员的显隐
	*
	* 5:根据tag来控制staticmesh的显隐和碰撞类型
	* 6:根据tag数组来控制staticmesh的显隐和碰撞类型
	*/

	//1:该函数根据tag来控制staticmeshactor的显隐，仅针对单个tag
		UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetHiddenbyTag(const UObject* WorldContextObject, FName Tag, bool bNewHidden);

	//2:该函数根据tag来控制staticmeshactor的显隐，针对的是复数tag
	UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetHiddenbyTags(const UObject* WorldContextObject, TArray<FName>Tags, bool bNewHidden);


	//3:该函数控制的是场景中单一类型Actor的显隐,根据判断的是actor的类型,不建议使用该函数控制staticmesh的显隐，因为考虑到了碰撞存在
	UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetClassHiddenInGame(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, bool bNewHidden);

	//4:该函数控制的是场景中多种类型Actor的显隐，但不改变其碰撞,根据判断的是外界传入的Actor类型数组,不建议使用该函数控制staticmesh的显隐，因为考虑到了碰撞存在
	UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetAllClassesHiddenInGame(const UObject* WorldContextObject, TArray<TSubclassOf<AActor>>ActorClasses, bool bNewHidden);


	//5:该函数在根据tag改变staticmesh显隐的同时，也能改变其预设碰撞类型，公开出去是为了设置成中文不然设置不了
	UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetHiddenAndCollisionbyTag(const UObject* WorldContextObject, FName Tag, FName ProfileName, bool bNewHidden);

	//6:该函数在根据tag数组改变staticmesh显隐的同时，也能改变其预设碰撞类型，公开出去是为了设置成中文不然设置不了
	UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetHiddenAndCollisionbyTags(const UObject* WorldContextObject, TArray<FName>Tags, FName ProfileName, bool bNewHidden);

	/*
	*材质改变类函数，该类功能目前仅控制的是staticmesh
	*/
	//1:该函数根据tag改变staticmesh的材质，默认改变的材质位置是0，适用于单一材质的物体
	UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetMaterialByTag(const UObject* WorldContextObject, FName Tag, int32 ElementIndex, UMaterialInterface* Material);

	//2:该函数根据tag数组给拥有不同tag的staticmesh改变材质，默认改变材质位置为0，适用于单一材质的物体
	UFUNCTION(BlueprintCallable, Category = "BIMLIB", meta = (WorldContext = "WorldContextObject"))
		static void BIM_SetMaterialByTags(const UObject* WorldContextObject, TArray<FName>Tags, int32 ElementIndex, UMaterialInterface* Material);

	
};
