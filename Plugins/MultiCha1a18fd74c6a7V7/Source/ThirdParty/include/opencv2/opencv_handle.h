#ifndef OPENCV_LOADER_EXPORT_H_
#define OPENCV_LOADER_EXPORT_H_

#define DLL_EXPORT __declspec(dllexport)

#include <string>
#include <functional>
#include <memory>

namespace cv
{
	enum EOpenCVEvent
	{
		OnVideoStoped,
		OnNeedReconnect,
		OnUpdate,
	};

	class DLL_EXPORT VideoBuffer
	{
	public:
		unsigned char* Buffer = nullptr;

		VideoBuffer(size_t Count);

		~VideoBuffer();
	};

	using FVideoOpenedDelegate = std::function<void(int Width, int Height, int Frame, std::shared_ptr<VideoBuffer> VideoBuffer)>;
	using FOpenCVEventDelegateOneParam = std::function<void(EOpenCVEvent)>;

	struct DLL_EXPORT VideoInfo
	{
		FVideoOpenedDelegate		 VideoOpenedDelegate = nullptr;
		FOpenCVEventDelegateOneParam OpenCVEventDelegate = nullptr;

		int AutoReconnectMaxCount = 0;
		int CustomFps = 24;
		bool bUseCustomFps = false;

		std::string VideoURL = "";
		void* Ptr = nullptr;
	};

	void DLL_EXPORT PlayVideo(cv::VideoInfo& VideoInfo);

	void DLL_EXPORT StopVideo(void* Ptr);

	void DLL_EXPORT Reconnect(void* Ptr);
}


#endif