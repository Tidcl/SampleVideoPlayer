#pragma once
#include <opencv2/opencv.hpp>
#include "VideoPlay/VideoDecoder.h"
#include "VideoPlay/PlayController.h"

//可以用来解码出gif和视频的关键帧，并缓存指定数量的帧和指定时长的帧到队列中
class EditDecoder : public VideoDecoder{
public:
	EditDecoder();
	virtual ~EditDecoder();

	void setStartPlayTime(int time);

	cv::Mat popFrontMat();

	double frontTimeStep();

	int m_width;
	int m_height;
protected:
	virtual void decode() override;
private:
	int m_startPlayTime = 0;
	bool m_isLoopDecode = false;
	std::mutex m_mutex;
};