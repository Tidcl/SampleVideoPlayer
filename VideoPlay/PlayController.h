/*
 * @Author: xctang xctang@163.com
 * @Date: 2024-08-18 11:41:28
 * @LastEditors: xctang xctang@163.com
 * @LastEditTime: 2024-08-19 16:01:35
 * @FilePath: \VideoEditor\VideoPlay\PlayController.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <windows.h>
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

#define Frame AVFrame

typedef void (frameRecall)(void*, void*, double playTimeMS);

enum PlayStatus {
	playing = 0x00,
	stop,
	pause
};

class PlayDecoder;

class PlayController
{
public:
    PlayController();
    ~PlayController();

    void setVideoUrl(std::string url) { m_url = url; };
    PlayStatus status();
    double playTimeSeconds();

    int startPlay();
    void stopPlay();

    void setPause(bool pause) { m_pause = pause; };
    bool pause() { return m_pause; };
    void setSeekTime(long timeMS); //设置跳转到指定ms

    double totalDuration();

    //播放速度
    double playSpeed() { return m_playSpeed; };
    void setPlaySpeed(double playSpeed);

    void setFrameRecall(frameRecall* func, void* v);

    //static void updatePlayTime(void* data);

    /// @brief 将视频帧保存为图片
    /// @param frame 视频帧
    /// @param path 保存的路径
    /// @param codec_ctx 
    void saveFrameToPng(AVFrame *frame, const char *path, AVCodecContext *codec_ctx);

    /// @brief 帧转图片
    /// @param frame [in]视频帧
    /// @param  [in]图片编码器ID,如jpg:AV_CODEC_ID_MJPEG，png:AV_CODEC_ID_PNG
    /// @param outbuf [out]图片缓存，由外部提供
    /// @param outbufSize [in]图片缓存长度
    /// @return 返回图片实际长度
    int frameToImage(AVFrame *frame, enum AVCodecID codecID, uint8_t *outbuf, size_t outbufSize, AVCodecContext *codec_ctx);

    /// @brief 压缩frame图片
    /// @param frame 视频帧
    /// @return 压缩后的视频帧
    AVFrame compressImage(AVFrame *frame);

    //使用条件变量给线程加锁等待
    void threadWait();

    ////解锁条件变量
    //void 

protected:
    void seek(long timeMS); //跳到指定ms

private:
    std::string m_url; // file url 资源路径支持本地文件、网络文件
    std::thread* m_playThread = nullptr;// thread 用于执行视频解码
    bool m_releaseThread;
    frameRecall* m_func; // 处理一帧的回调函数
    void* m_v;  //回调函数参数

    bool m_pause = false;   //是否暂停
    bool m_stop = true; //是否停止

    double m_playStartTime = 0;
    double m_playTime = 0; //当前播放到的时间。单位毫秒
    double m_playSpeed = 1; //播放速度
    double m_pauseMS = 0;
    double m_totalDuration = 0;

    PlayDecoder* m_playDecoder = nullptr;   //解码器。用于获取显示用的视频帧和音频帧
    std::chrono::time_point<std::chrono::steady_clock> m_playStart; //开始播放的时间戳
    long m_seekTimeMs = 0;  //播放偏移时间戳

    friend class PlayDecoder;

    //做同步控制
    std::mutex m_syncMutex;
    std::condition_variable m_syncCond;
    bool m_seekFlag = false;
    //long m_seekTime;
};


