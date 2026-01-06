// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/UIManager.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "Common/StructDefine.h"
#include "UI/Base_UI.h"
#include "Func/ZHJZ_Interface.h"


UBase_UI* UUIManager::OpenUI(const FString& UIName)
{
	UBase_UI* widget = nullptr;

	FString Text;

	//关闭之前打开的ui
	if (CurrentWidgetMes.CurrentWidget && CurrentWidgetMes.UIName != "MainUI")
	{
		if (CurrentWidgetMes.CurrentWidget->GetClass()->ImplementsInterface(UZHJZ_Interface::StaticClass()))
		{
			IZHJZ_Interface::Execute_StopMedia(CurrentWidgetMes.CurrentWidget);

		}
		CloseUI(CurrentWidgetMes.UIName,true);
	}

	if (!CurrentUIMap.Contains(UIName) || !CurrentUIMap[UIName])
	{
		widget = InitUI(UIName);

		//Text = FString::Printf(TEXT("1 OpenUI-------UIName: %s"), *UIName);
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Text);

		if (widget)
		{
			widget->AddToViewport();
			FCurrentUIMes Mes;
			Mes.CurrentWidget = widget;
			Mes.UIName = UIName;
			CurrentWidgetMes = Mes;

			//Text = FString::Printf(TEXT("2 OpenUI-------widget: %s"), *widget->GetName());
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Text);
		}
	}
	else
	{
		CurrentUIMap[UIName]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		FCurrentUIMes Mes;
		Mes.CurrentWidget = CurrentUIMap[UIName];
		Mes.UIName = UIName;
		CurrentWidgetMes = Mes;

		//Text = FString::Printf(TEXT("3 OpenUI-------Mes.UIName: %s"), *Mes.UIName);
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Text);
	}

	if (CurrentUIMap.Contains(UIName))
	{
		UBase_UI* pBase = CurrentUIMap[UIName];

		//Text = FString::Printf(TEXT("4 OpenUI-------UIName: %s"), *UIName);
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Text);


		return pBase;
	}
	else
	{
		//Text = FString::Printf(TEXT("5 OpenUI-------UIName: %s"), *UIName);
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, Text);
		return nullptr;
	}
}

bool UUIManager::CloseUI(const FString& UIName, bool destory)
{
	UBase_UI* widget = nullptr;
	FString uiname = UIName;
	//传空默认关闭当前UI
	if (uiname == "")
	{
		uiname = CurrentWidgetMes.UIName;

	}

	//检测UIWidgetMap是否包含UIName
	if (CurrentUIMap.Contains(uiname))
	{
		widget = CurrentUIMap[uiname];
	}

	if (nullptr == widget)
	{
		return false;
	}

	if (!destory)
	{
		widget->SetVisibility(ESlateVisibility::Hidden);
		CurrentWidgetMes = FCurrentUIMes();
	}
	else
	{
		widget->RemoveFromParent();
		widget->ConditionalBeginDestroy();
		CurrentUIMap[uiname] = nullptr;

		CurrentWidgetMes = FCurrentUIMes();
	}
	return true;

}

UBase_UI* UUIManager::GetUI(const FString& UIName)
{
	UBase_UI* widget = nullptr;
	if (!CurrentUIMap.Contains(UIName))
	{
		return nullptr;
	}
	widget = CurrentUIMap[UIName];
	if (nullptr == widget)
	{
		return nullptr;
	}
	return widget;
}

UBase_UI* UUIManager::CreateUI(const FString& UIName)
{
	UBase_UI* widget = nullptr;

	if (UIName == "")
	{
		return widget;
	}

	widget = InitUI(UIName, true);

	if (widget)
	{
		widget->AddToViewport();
	}

	return widget;
}


bool UUIManager::CheckUI(const FString& UIName)
{
	if (!UIPathMap.Contains(UIName))
	{
		return false;
	}
	return true;
}

UBase_UI* UUIManager::InitUI(const FString& UIName, bool isDestroy)
{

	FString Text;
	if (!CheckUI(UIName))
	{

		Text = FString::Printf(TEXT("1 InitUI---  UIPathMap is null----UIName: %s"), *UIName);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);
		
		return nullptr;
	}
	FString path = UIPathMap[UIName];

	if (path.IsEmpty())
	{
		Text = FString::Printf(TEXT("2 InitUI--- path.IsEmpty()----UIName: %s"), *UIName);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);
		return nullptr;
	}
	TSubclassOf<UBase_UI> UICurrent = LoadClass<UBase_UI>(nullptr, *path);
	if(UICurrent == nullptr)
	{
		Text = FString::Printf(TEXT("3 InitUI--- UICurrent is null----UIName: %s"), *UIName);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);
		return nullptr;
	}
	APlayerController* playerContro = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!playerContro)
	{
		Text = FString::Printf(TEXT("3 InitUI--- playerContro is null----UIName: %s"), *UIName);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);
		return nullptr;
	}

	UBase_UI* CurrentUI = CreateWidget<UBase_UI>(playerContro, UICurrent);
	if (CurrentUI && !isDestroy)
	{
		CurrentUIMap.Add(UIName, CurrentUI);
		//Text = FString::Printf(TEXT("4 InitUI-------UIName: %s"), *UIName);
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);
	}
	//Text = FString::Printf(TEXT("5 InitUI-------CurrentUI: %s  isDestroy:%i "), *CurrentUI->GetName(), isDestroy);
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, Text);
	return CurrentUI;
}