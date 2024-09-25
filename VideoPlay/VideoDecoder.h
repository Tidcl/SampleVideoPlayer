#pragma once
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}
#include <iostream>
#include <vector>
#include <deque>
#include <mutex>
#include <memory>
#include <windows.h>
#include <chrono>
#include <thread>
#include <functional>

class PlayController;
//用于将解复用器中解码的视频帧和音频帧放入处理队列
//播放解码器
class VideoDecoder : public std::enable_shared_from_this<VideoDecoder> {
public:
	friend class PlayController;

	VideoDecoder();
	~VideoDecoder();

	void setPlayController(std::shared_ptr<PlayController> controller);
	void initDecode(std::string url);
	void freeDecode();
	void startDecode();
	void stopDecode();
	void videoSeek(long timeMs); //跳转到指定播放时间位置解码 ms
	bool isFree();

	std::deque<AVFrame*>& videoFrameVector() { return m_videoFrameVec; };
	std::deque<AVFrame*>& audioFrameVector() { return m_audioFrameVec; };
	void freeBuffer();

	double videoTimeBaseMs() { return (video_time_base.num * 1.0 / video_time_base.den) * 1000; };
	double audioTimeBaseMs() { return (audio_time_base.num * 1.0 / audio_time_base.den) * 1000; };
protected:
	virtual void decode();
protected:
	std::thread* m_decodeThread = nullptr;
	bool m_stopDecode;

	std::shared_ptr<PlayController> m_playController = nullptr;//

	AVFormatContext* m_formatContext = nullptr;
	//视频解码相关对象
	AVCodecParameters* video_codecpar = nullptr;
	AVCodec* video_codec = nullptr;
	AVCodecContext* video_context = nullptr;
	AVRational video_time_base;
	int video_stream_index = -1;
	//音频解码相关对象
	AVCodecParameters* audio_codecpar = nullptr;
	AVCodec* audio_codec = nullptr;
	AVCodecContext* audio_context = nullptr;
	AVRational audio_time_base;
	int audio_stream_index = -1;

	int playTime = 0;

	//帧队列至少缓存5s的数据
	std::deque<AVFrame*> m_videoFrameVec;//视频帧队列
	std::deque<AVFrame*> m_audioFrameVec;//音频帧队列

	long m_bufferTime = 0;//缓存多少ms的帧
	long m_bufFrameCount = 300;	//缓存多少个帧

	std::mutex m_mutex;	//队列同步锁
	bool m_seekFlag = false;	//seek标志，如果为true表示进入seek状态
};
