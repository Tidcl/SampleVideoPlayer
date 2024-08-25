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

class PlayController;
//用于将解复用器中解码的视频帧和音频帧放入处理队列
//播放解码器
class PlayDecoder {
public:
	friend class PlayController;

	PlayDecoder() = default;
	~PlayDecoder() = default;

	void setPlayController(PlayController* controller) { m_playController = controller; };
	void initDecode(std::string url);
	void freeDecode();
	void startDecode();
	void stopDecode();
	void videoSeek(long timeMs); //跳转到指定播放时间位置解码 ms
	bool isFree();

	std::deque<AVFrame*>& videoFrameVector() { return m_videoFrameVec; };
	std::deque<AVFrame*>& audioFrameVector() { return m_audioFrameVec; };
	void freeBuffer();

	double videoTimeBase() { return (video_time_base.num * 1.0 / video_time_base.den) * 1000; };
	double audioTimeBase() { return (audio_time_base.num * 1.0 / audio_time_base.den) * 1000; };
private:
	std::thread* m_decodeThread;
	bool m_stopDecode;

	PlayController* m_playController;//

	AVFormatContext* m_formatContext;
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

	long m_bufferTime = 0;//缓存多少ms

	std::mutex m_mutex;
};
