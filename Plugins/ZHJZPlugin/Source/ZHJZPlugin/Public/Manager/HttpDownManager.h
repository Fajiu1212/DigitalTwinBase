// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "HttpDownManager.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDownloadDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDownloadDelegate, float, Progress);

UENUM(BlueprintType)
enum class EDownState : uint8
{
	_Ready,
	_Downloading,
	_Succees,
	_Failed,
	_Cancelled,
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ZHJZPLUGIN_API UHttpDownManager : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	/**
	 * 下载文件 
	 * url :下载地址
	 * directory： 存储地址 例如：FPaths::ProjectSavedDir() + "PersistentDownloadDir/PakCache/";
	 */ 
	UFUNCTION(BlueprintCallable, Category = "AsyncHttp", meta = (BlueprintInternalUseOnly = "true"))
		static UHttpDownManager* AsyncDownloadHttp(const FString& url, const FString& directory, const FString& filename ,UHttpDownManager*& httpObject);

		// 取消当前下载 
	UFUNCTION(BlueprintCallable, Category = "HTTP Download")
		void CancelDownload();
	
	//下载包
	void DownloadPak(FString URL);

	void HandleDownloadPakRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

	//处理下载进度
	void HandleHttpRequestPakProgress(FHttpRequestPtr HttpRequest, uint64 BytesSent, uint64 BytesReceived);

public:

	//输出节点
	UPROPERTY(BlueprintAssignable)
		FDownloadDelegate OnSucc;

	//输出节点
	UPROPERTY(BlueprintAssignable)
		FDownloadDelegate OnFailed;

	UPROPERTY(BlueprintAssignable)
		FDownloadDelegate OnProgress;

	UPROPERTY(BlueprintAssignable)
		FDownloadDelegate OnCancelled;

	//任务状态
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownManager")
		EDownState	State;

	//下载地址
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownManager")
		FString	Url;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		FString	Directory;//文件路径

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
		FString	FileName;//文件名

	//下载进度
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownManager")
		float Progress;

	//当前下载了多少个字节了
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownManager")
		int32 CurFileSize;

	//http文件的总大小
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownManager")
		int64	TotalFileSize;

protected:

	// 当前活动的HTTP请求
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;

};
