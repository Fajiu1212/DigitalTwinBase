// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/HttpDownManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "ChunkDownloader.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
#include "Runtime/PakFile/Public/IPlatformFilePak.h"
#include "Kismet/KismetStringLibrary.h"

// 异步下载
UHttpDownManager* UHttpDownManager::AsyncDownloadHttp(const FString& url, const FString& directory, const FString& filename, UHttpDownManager*& httpObject)
{
	UHttpDownManager* AsyncHttpObject = NewObject<UHttpDownManager>();
	AsyncHttpObject->Directory = directory;
	AsyncHttpObject->FileName = filename;
	AsyncHttpObject->DownloadPak(url);
	httpObject = AsyncHttpObject;
	return httpObject;
}

void UHttpDownManager::CancelDownload()
{
	if (State == EDownState::_Ready || State == EDownState::_Downloading)
	{
		// 取消HTTP请求
		if (CurrentRequest.IsValid())
		{
			CurrentRequest->CancelRequest();
			CurrentRequest.Reset();
		}

		State = EDownState::_Cancelled;

		// 触发取消委托
		if (OnCancelled.IsBound())
		{
			OnCancelled.Broadcast(Progress);
		}
	}
}


void UHttpDownManager::DownloadPak(FString URL)
{
	//TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	//HttpRequest->OnProcessRequestComplete().BindUObject(this, &UHttpDownManager::HandleDownloadPakRequest);
	//HttpRequest->OnRequestProgress64().BindUObject(this, &UHttpDownManager::HandleHttpRequestPakProgress);
	//HttpRequest->SetURL(URL);
	//HttpRequest->SetVerb(TEXT("GET"));
	//HttpRequest->ProcessRequest();

	//State = EDownState::_Ready;
	//Url = URL;

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	// 绑定完成回调，确保参数匹配
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UHttpDownManager::HandleDownloadPakRequest);
	// 绑定进度回调，参数使用 uint64
	HttpRequest->OnRequestProgress64().BindUObject(this, &UHttpDownManager::HandleHttpRequestPakProgress);
	HttpRequest->SetURL(URL);
	HttpRequest->SetVerb(TEXT("GET"));
	HttpRequest->ProcessRequest();

	State = EDownState::_Ready;
	Url = URL;
}

void UHttpDownManager::HandleDownloadPakRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (State == EDownState::_Cancelled)
	{
		return;
	}

	if (!HttpRequest.IsValid() || !HttpResponse.IsValid())
	{
		// 处理取消后的回调
		if (State == EDownState::_Cancelled)
		{
			return;
		}

		State = EDownState::_Failed;
		if (OnFailed.IsBound())
		{
			OnFailed.Broadcast(Progress);
		}
		return;
	}

	int32 responseCode = HttpResponse->GetResponseCode();

	// 处理取消请求的情况
	if (HttpRequest->GetStatus() == EHttpRequestStatus::Failed ||
		HttpRequest->GetStatus() == EHttpRequestStatus::Failed_ConnectionError)
	{
		// 如果是用户取消，状态已经是_Cancelled，这里不再处理
		if (State != EDownState::_Cancelled)
		{
			State = EDownState::_Failed;
			if (OnFailed.IsBound())
			{
				OnFailed.Broadcast(Progress);
			}
		}
		return;
	}

	if (bSucceeded && EHttpResponseCodes::IsOk(responseCode))
	{
		FFileHelper::SaveArrayToFile(HttpResponse->GetContent(), *FString::Printf(TEXT("%s%s"), *Directory, *FileName));

		if (State == EDownState::_Downloading)
		{
			State = EDownState::_Succees;
		}

		if (OnSucc.IsBound())
		{
			OnSucc.Broadcast(Progress);
		}
	}
	else
	{
		State = EDownState::_Failed;

		if (OnFailed.IsBound())
		{
			OnFailed.Broadcast(Progress);
		}
	}
}

void UHttpDownManager::HandleHttpRequestPakProgress(FHttpRequestPtr HttpRequest, uint64 BytesSent, uint64 BytesReceived)
{
	if (State == EDownState::_Cancelled)
	{
		return;
	}

	if (State == EDownState::_Ready)
	{
		State = EDownState::_Downloading;
	}

	// 获取文件大小
	if (HttpRequest->GetResponse().IsValid())
	{
		TotalFileSize = HttpRequest->GetResponse()->GetContentLength();
	}

	CurFileSize = BytesReceived;

	if (TotalFileSize > 0)
	{
		Progress = static_cast<float>(CurFileSize) / static_cast<float>(TotalFileSize);
	}
	else
	{
		Progress = 0.0f;
	}

	if (OnProgress.IsBound())
	{
		OnProgress.Broadcast(Progress);
	}
	 
	// HttpRequest.Get();
    //FString Text = FString::Printf(TEXT("HandleHttpRequestPakProgress +++++++ %i  %i"), TotalFileSize, BytesReceived);
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, Text);
}
