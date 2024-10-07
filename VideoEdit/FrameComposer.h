#pragma once

#include <opencv2/opencv.hpp>
#include "VideoPlay/VideoTimer.h"
#include "FramePusher.h"
#include "EditDecoder.h"

enum SourceType {
	Picture = 0,
	Gif,
	Video
};

class ImgSource {	//图源
public:
	ImgSource();
	~ImgSource();

	cv::Mat& getMat();

	cv::Mat& getPointTimeMat(double time);

	void initSource();

	std::string m_url;//图源路径
	cv::Mat m_imgMat;	//图源矩阵，合成时使用
	int m_width = 0;	//长
	int m_height = 0;	//宽
	int m_x = 0;		//x位置
	int m_y = 0;		//y位置
	int m_fps = 0;

	double m_startPlayTime;	//开始播放时间
	double m_pauseTime;	//暂停的时间

	SourceType m_type;
	bool m_pauseFlag = false;
	cv::Mat m_lastVideoMat;
	//来自Decoder的帧
	std::shared_ptr<EditDecoder> m_decoder = nullptr;
};

class FrameComposer {
public:
	FrameComposer();
	~FrameComposer();

	void setSize(int width, int height);
	void setPusher(std::shared_ptr<FramePusher> pusher);
	void setShowCallback(void(func)(cv::Mat, void*), void* component);

	void addImgSource(std::shared_ptr<ImgSource> imgSource);
	void removeImgSource(int index);
	int imgSourceCount();
	std::shared_ptr<ImgSource> imgSource(int index);
	void startCompose();
	void stopCompose();
protected:
	void composeFrameFromSource();
private:
	std::shared_ptr<FramePusher> m_framePusher = nullptr;
	std::vector<std::shared_ptr<ImgSource>> m_imgSourceVec;

	void(*m_func)(cv::Mat, void*) = nullptr;
	void* m_funcVal = nullptr;

	VideoTimer m_composeTimer;
	int m_composeFPS = 60;
	int width = 1280;
	int height = 720;
};