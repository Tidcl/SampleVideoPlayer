#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
//#include <libswscale/swscale_internal.h>
}
#include <memory>
#include <opencv2/opencv.hpp>

class FramePusher {
public:
	FramePusher();
	~FramePusher();
	void updateFrame(cv::Mat mat);
	int openCodec(int width = 1280, int height = 720, int fps = 24);
	void startPush();
	void stopPush();
private:
	int pushing();

	void resizeSws(cv::Mat& img);
private:
	bool m_stopPullFlag = false; //停止推流标志
	std::shared_ptr<std::thread> m_pullThread = nullptr;	//推流线程

	std::string m_serverPath = "rtmp://127.0.0.1/live/livestream";	//推流地址
	int m_width = 1280;	//推流帧宽度
	int m_height = 720;	//推流帧高度
	int m_frameRate = 60;	//推流帧率

	cv::Mat m_updateMat;	//用来更新推送mat
	bool m_updateFlag = false;		//mat更新标志位

	std::shared_ptr<AVFormatContext> m_fmt_ctx = nullptr;//解码器上下文
	std::shared_ptr<AVCodecContext> m_codec_ctx = nullptr;//格式化上下文
	std::shared_ptr<SwsContext> m_sws_ctx = nullptr;
};