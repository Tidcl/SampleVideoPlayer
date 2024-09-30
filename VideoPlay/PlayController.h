#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
//#include <windows.h>
#include <chrono>
#include <thread>
#include <FL/Fl.H>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}
#include <zlib.h>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <memory>
//#include "VideoDecoder.h"
#include "VideoTimer.h"

#define Frame AVFrame


/// @brief 将视频帧保存为图片
/// @param frame 视频帧
/// @param path 保存的路径
/// @param codec_ctx 
void saveFrameToPng(AVFrame* frame, const char* path, AVCodecContext* codec_ctx);

/// @brief 帧转图片
/// @param frame [in]视频帧
/// @param  [in]图片编码器ID,如jpg:AV_CODEC_ID_MJPEG，png:AV_CODEC_ID_PNG
/// @param outbuf [out]图片缓存，由外部提供
/// @param outbufSize [in]图片缓存长度
/// @return 返回图片实际长度
int frameToImage(AVFrame* frame, enum AVCodecID codecID, uint8_t* outbuf, size_t outbufSize, AVCodecContext* codec_ctx);

typedef void (frameRecall)(void*, void*, double playTimeMS);

enum PlayStatus {
	playing = 0x00,
	stop,
	pause
};

class VideoDecoder;

class PlayController : public std::enable_shared_from_this<PlayController>
{
public:
    PlayController();
    ~PlayController();

    void setVideoUrl(std::string url);
	int startPlayByTimer();
	void stopPlayByTimer();
    PlayStatus status();
    
    static void timerCallBack(void* val);   
    void setPause(bool pause) { m_pause = pause; m_timer.setPause(m_pause); };
    bool pause() { return m_pause; };
    void setSeekTime(long timeMS); //设置跳转到指定ms
    double totalDuration();

    void setPlaySpeed(double playSpeed);    //播放速度
    void setFrameRecall(frameRecall* func, void* v);

    AVFrame& lastFrame();

    //使执行该函数的解码线程阻塞等待播放线程
    void threadWait();

protected:
    void seek(long timeMS); //跳到指定ms

private:
    std::string m_url; // file url 资源路径支持本地文件、网络文件
    frameRecall* m_func = nullptr; // 处理一帧的回调函数
    void* m_v;  //回调函数参数

    bool m_pause = false;   //是否暂停
    bool m_stop = true; //是否停止

	bool m_seekFlag = false;
    double m_playSpeed = 1; //播放速度
    double m_totalDuration = 0;
    long m_seekTimeMs = 0;  //播放偏移时间戳

    std::shared_ptr<VideoDecoder> m_playDecoder;   //解码器。用于获取显示用的视频帧和音频帧

    //做同步控制
    bool m_isCodecoWaited = false;  //编码器是否进入等待状态

    //定时器间隔
    VideoTimer m_timer;
    double m_timerIntervalMS = 1000.0/30;

    AVFrame m_lastFrame; //将最新的一帧放入此变量

    friend class VideoDecoder;
};


