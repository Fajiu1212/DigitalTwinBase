// Copyright CengJia.  Year : 2022

#pragma once

#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Windows/WindowsPlatformProcess.h"

class FOpenCVPlayerModule : public IModuleInterface
{
	void* OpenCvDllHandle = nullptr;
	void* OpenCvVideoDllHandle = nullptr;
	void* OpenCvMsmfDllHandle = nullptr;
	void* openCvhandle = nullptr;


public:

	virtual void StartupModule() override
	{
		const FString DllPath = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("OpenCVPlayer"))->GetBaseDir() + "/Source/ThirdParty/dll/");
		
		OpenCvVideoDllHandle = FPlatformProcess::GetDllHandle(*(DllPath + "opencv_videoio_ffmpeg490_64.dll"));
		OpenCvDllHandle = FPlatformProcess::GetDllHandle(*(DllPath + "opencv_world490.dll"));
		OpenCvMsmfDllHandle = FPlatformProcess::GetDllHandle(*(DllPath + "opencv_videoio_msmf490_64.dll"));
		openCvhandle = FPlatformProcess::GetDllHandle(*(DllPath + "opencv_handle.dll"));
	}

	virtual void ShutdownModule() override
	{
		if (OpenCvDllHandle)
		{
			FPlatformProcess::FreeDllHandle(OpenCvDllHandle);
			OpenCvDllHandle = nullptr;
		}
		if (OpenCvVideoDllHandle)
		{
			FPlatformProcess::FreeDllHandle(OpenCvVideoDllHandle);
			OpenCvVideoDllHandle = nullptr;
		}
		if (OpenCvMsmfDllHandle)
		{
			FPlatformProcess::FreeDllHandle(OpenCvMsmfDllHandle);
			OpenCvMsmfDllHandle = nullptr;
		}
		if (openCvhandle)
		{
			FPlatformProcess::FreeDllHandle(openCvhandle);
			openCvhandle = nullptr;
		}
	}

};


IMPLEMENT_MODULE(FOpenCVPlayerModule, OpenCVPlayer)
