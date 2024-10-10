#include "FrameComposer.h"
#include "EditPanel.h"

FrameComposer::FrameComposer()
{

}

FrameComposer::~FrameComposer()
{
	m_composeTimer.stop();
}

void FrameComposer::setSize(int width, int height)
{
	this->width = width;
	this->height = height;
}

void FrameComposer::setPusher(std::shared_ptr<FramePusher> pusher)
{
	m_framePusher = pusher;
	m_composeFPS = pusher->FPS();
}

void FrameComposer::setShowCallback(void(func)(cv::Mat, void*), void* component)
{
	m_func = func;
	m_funcVal = component;
}

void FrameComposer::setPanel(EditPanel* panel)
{
	m_panel = panel;
}

void FrameComposer::addImgSource(std::shared_ptr<ImgSource> imgSource)
{
	m_imgSourceVec.push_back(imgSource);
}

void FrameComposer::removeImgSource(int index)
{
	if (index == -1) return;
	auto ptr = (*(m_imgSourceVec.begin() + index))->m_decoder;
	if (ptr) { ptr->stopDecode(); }
	m_imgSourceVec.erase(m_imgSourceVec.begin() + index);
}

int FrameComposer::imgSourceCount()
{
	return m_imgSourceVec.size();
}

std::shared_ptr<ImgSource> FrameComposer::imgSource(int index)
{
	if (index < 0) return nullptr;
	return m_imgSourceVec.at(index);
}

void FrameComposer::startCompose()
{
	m_composeTimer.setInterval(10);
	m_composeTimer.setCallbackFun([](void* val) {
		FrameComposer* ep = (FrameComposer*)val;
		ep->composeFrameFromSource();
		//从帧队列中取出
		}, this);
	m_composeTimer.start();
}

void FrameComposer::stopCompose()
{
	m_composeTimer.stop();
}

void FrameComposer::composeFrameFromSource()
{
	static double frameTime = 1000.0 / m_composeFPS;
	static double decodeFrameMS = 0;
	static double playFrameMS = 0;
	static double bufFrameCount = 10;

	double nowTime = m_composeTimer.timeMS();
	cv::Mat bufferFrame(height, width, CV_8UC3);

	//遍历图源合成到合成帧中
	for (auto imgSour : m_imgSourceVec)
	{
		if (imgSour)
		{
			cv::Mat& mat = imgSour->getPointTimeMat(decodeFrameMS);
			//将图源重新缩放
			if (mat.rows == 0)
				continue;
			//如果焦点区域超出就需要裁剪原图
			mat.copyTo(bufferFrame(cv::Rect(imgSour->m_x, imgSour->m_y, imgSour->m_width, imgSour->m_height)));
		}
	}
	
	//if (m_framePusher->bufferCount() >= bufFrameCount)
	//{
	//	return;
	//}

	//if (m_framePusher && m_framePusher->bufferCount() <= bufFrameCount)
	//{
	////	//将帧发送给推流器
	//	m_framePusher->pushFrameToVec(bufferFrame);
	//}

	if (nowTime > playFrameMS)
	{
		m_func(bufferFrame, m_funcVal);
		playFrameMS += 200;	//5fps预览
	}

	if (nowTime > decodeFrameMS)
	{
		decodeFrameMS += frameTime;
		if (m_framePusher && m_framePusher->bufferCount() <= bufFrameCount)
		{
			m_framePusher->pushFrameToVec(bufferFrame);
		}
	}
		
}
