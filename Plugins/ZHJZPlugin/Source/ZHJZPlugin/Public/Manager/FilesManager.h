// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "FilesManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDownloadProgressDelegate, const int32&, ReceivedDataInBytes, const int32&, TotalDataInBytes, const TArray<uint8>&, BinaryData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDownloadDelegates);

/**
 * 
 */
UCLASS(Blueprintable)
class ZHJZPLUGIN_API UFilesManager : public UObject
{
	GENERATED_BODY()

public:

	//读取文件
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static FString ReadStringToFile(FString Dir);

	//写入文件
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void WriteStringToFile(FString FileName, FString content);

	//查找文件
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static TArray<FString> FindFiles(FString Path);

	//修改文件路径
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool MoveFileTo(FString To, FString From);

	//删除文件
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static bool DeleteFile(FString FilePath);

	//打开指定文件
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void OpenFile(FString FilePath);

	//创建文件夹
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void CreateDic(FString filePath);

	//删除文件夹
	UFUNCTION(BlueprintCallable, Category = "File operation")
		static void DeleteDic(FString filePath);

	//打开文件夹
	UFUNCTION(BlueprintCallable, Category = "File operation")
		void OpenWindowsFiles(FString InUNCPath, FString& OutPath);




	// 异步下载文件 
	UFUNCTION(BlueprintCallable, Category = "AsyncHttp", meta = (BlueprintInternalUseOnly = "true"))
	static UFilesManager* AsyncHttpDownload(const FString& url); 
	
	void DownloadRequestStart(const FString& url);

	void DownloadRequestHandle(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful);

	void DownloadProgress(FHttpRequestPtr request, uint64 bytesSent, uint64 bytesReceived);
	//输出节点 
	UPROPERTY(BlueprintAssignable)
	FDownloadProgressDelegate OnProgressUpdate; 
	//最好把有参数的放在前面，不然参数在蓝图节点有时候被吃掉了 
	//输出节点
	UPROPERTY(BlueprintAssignable)
		FDownloadDelegates OnSucc;
	//输出节点
	UPROPERTY(BlueprintAssignable)
		FDownloadDelegates OnFailed;

};
