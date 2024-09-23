#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
}
#include <memory>
#include <opencv2/opencv.hpp>

class FramePusher {
public:
	FramePusher();
	~FramePusher();
	void updateMat(cv::Mat mat);
	int initFFmpeg();
	void startPush();
	void stopPush();
private:
	int pushing();
private:
	bool m_stopPullFlag = false; //ֹͣ������־
	std::shared_ptr<std::thread> m_pullThread = nullptr;	//�����߳�

	//std::string m_serverPath = "rtmp://127.0.0.1:1935/testlive";	//������ַ
	std::string m_serverPath = "rtmp://127.0.0.1/live/livestream";	//������ַ
	int m_width = 400;	//����֡���
	int m_height = 270;	//����֡�߶�
	int m_bitRate = 400000;	//����������
	int m_frameRate = 30;	//����֡��
	cv::Mat m_updateMat;	//������������mat
	bool m_updateFlag = false;		//mat���±�־λ

	std::shared_ptr<AVFormatContext> m_fmt_ctx = nullptr;//������������
	std::shared_ptr<AVCodecContext> m_codec_ctx = nullptr;//��ʽ��������
};