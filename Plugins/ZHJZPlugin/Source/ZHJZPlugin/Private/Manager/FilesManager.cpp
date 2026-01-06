// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/FilesManager.h"


#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "ChunkDownloader.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Runtime/PakFile/Public/IPlatformFilePak.h"
#include "Kismet/KismetStringLibrary.h"


FString UFilesManager::ReadStringToFile(FString Dir)
{
	FString Result;
	FFileHelper::LoadFileToString(Result, *Dir);
	return Result;
}

void UFilesManager::WriteStringToFile(FString FileName, FString content)
{
	FFileHelper::SaveStringToFile(content, *FileName);
}


TArray<FString> UFilesManager::FindFiles(FString Path)
{
	TArray<FString> FilePathList;
	FilePathList.Empty();
	//FFileManagerGeneric::Get().FindFilesRecursive(FilePathList, *Path, *Filter, true, true);
	FFileManagerGeneric::Get().FindFiles(FilePathList, *Path);

	return FilePathList;
}

bool UFilesManager::MoveFileTo(FString To, FString From)
{
	return IFileManager::Get().Move(*To, *From);
}

bool UFilesManager::DeleteFile(FString FilePath)
{
	return IFileManager::Get().Delete(*FilePath);
}

void UFilesManager::OpenFile(FString FilePath)
{
	FPlatformProcess::CreateProc(*FilePath, nullptr, true, false, false, nullptr, 0, nullptr, nullptr);
}

void UFilesManager::CreateDic(FString filePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectory(*filePath);
}

void UFilesManager::DeleteDic(FString filePath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteDirectory(*filePath);
}

void UFilesManager::OpenWindowsFiles(FString InUNCPath, FString& OutPath)
{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		DesktopPlatform->OpenDirectoryDialog(
			nullptr,
			TEXT("Open Directory"),
			InUNCPath,
			OutPath
		);	
}


// 异步下载
UFilesManager* UFilesManager::AsyncHttpDownload(const FString& url)
{
	UFilesManager* AsyncHttpObject = NewObject<UFilesManager>();
	AsyncHttpObject->DownloadRequestStart(url); 
	return AsyncHttpObject; 
} 


void UFilesManager::DownloadRequestStart(const FString& url)
{ 
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> request = FHttpModule::Get().CreateRequest();
	request->SetVerb("GET");
	request->SetURL(url);
	request->OnProcessRequestComplete().BindUObject(this, &UFilesManager::DownloadRequestHandle);
	//请求回调 
	request->OnRequestProgress64().BindUObject(this, &UFilesManager::DownloadProgress);

	// 下载进度 
	request->ProcessRequest();

	// 手动GC 
	RemoveFromRoot();

	
} 

void UFilesManager::DownloadRequestHandle(FHttpRequestPtr request, FHttpResponsePtr response, bool bWasSuccessful) 
{ 
	RemoveFromRoot(); 
	if (bWasSuccessful && response.IsValid() && EHttpResponseCodes::IsOk(response->GetResponseCode()))
	{
		if (response->GetContentLength() > 0)
		{ 
			TArray<uint8> EmptyData; 
			OnProgressUpdate.Broadcast(response->GetContentLength(), response->GetContentLength(), EmptyData);
		} 
		FString FileSavePath = FPaths::ProjectDir() + "/download/gameplay.png";
		FString Path, Filename, Extension; 
		FPaths::Split(FileSavePath, Path, Filename, Extension); 
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile(); 
		if (!PlatformFile.DirectoryExists(*Path))
		{
			if (!PlatformFile.CreateDirectoryTree(*Path)) 
			{ 
				UE_LOG(LogTemp, Error, TEXT("Create Directory Failed!")); 
				OnFailed.Broadcast(); 
				return;
			}
				
		} 
		IFileHandle* FileHandle = PlatformFile.OpenWrite(*FileSavePath); 
		if (FileHandle) 
		{
			FileHandle->Write(response->GetContent().GetData(), response->GetContentLength()); 
			delete FileHandle; 
			OnSucc.Broadcast(); 
			return; 
		} 
		else 
		{ 
			UE_LOG(LogTemp, Error, TEXT("Save File Failed!")); 
			OnFailed.Broadcast();
			return; 
		} 
	} 
	UE_LOG(LogTemp, Error, TEXT("download File Failed!")); 
	OnFailed.Broadcast(); 
	return; 
}


// 下载进度
void UFilesManager::DownloadProgress(FHttpRequestPtr request, uint64 bytesSent, uint64 bytesReceived)
{
	if (request->GetResponse()->GetContentLength() > 0) 
	{
		TArray<uint8> EmptyData;
		OnProgressUpdate.Broadcast(bytesReceived, request->GetResponse()->GetContentLength(),EmptyData);
	} 
}

