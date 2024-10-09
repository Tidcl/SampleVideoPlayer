#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
//#include <libswscale/swscale_internal.h>
}
#include <memory>
#include <opencv2/opencv.hpp>

//֡������
class FramePusher {
public:
	FramePusher();
	~FramePusher();
	void setPushFPS(int fps);
	int FPS();

	void updateFrame(cv::Mat& mat);
	void pushFrame(cv::Mat& mat);
	int bufferCount();
	void clearBuffer();

	int openCodec(int width = 1280, int height = 720, int fps = 30);
	void closeCodec();

	void startPush();
	void stopPush();

	bool isInitSuccessful();
private:
	int pushing();

	void resizeSws(cv::Mat& img);
private:
	std::vector<cv::Mat> m_pushVector;

	bool m_stopPullFlag = false; //ֹͣ������־
	std::shared_ptr<std::thread> m_pullThread = nullptr;	//�����߳�

	std::string m_serverPath = "rtmp://127.0.0.1/live/livestream";	//������ַ
	int m_width = 1280;	//����֡���
	int m_height = 720;	//����֡�߶�
	int m_frameRate = 30;	//����֡��
	int m_bitRate = 64000 * 1000;

	cv::Mat m_updateMat;	//������������mat
	bool m_updateFlag = false;		//mat���±�־λ

	std::shared_ptr<AVFormatContext> m_fmt_ctx = nullptr;//������������
	std::shared_ptr<AVCodecContext> m_codec_ctx = nullptr;//��ʽ��������
	std::shared_ptr<SwsContext> m_sws_ctx = nullptr;
	AVStream* m_videoStream = nullptr;

	std::mutex m_mutex;
	bool m_initSuccessful = false;
};