#pragma once
#include <opencv2/opencv.hpp>
#include "VideoPlay/VideoDecoder.h"
#include "VideoPlay/PlayController.h"

//�������������gif����Ƶ�Ĺؼ�֡��������ָ��������֡��ָ��ʱ����֡��������
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