#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "FrameComposer.h"
#include "MoveLabel.h"
#include "FramePusher.h"
#include "VideoPlay/PlayController.h"
#include "VideoPlay/VideoTimer.h"
#include "EditDecoder.h"

class EditPanel : public Fl_Group {
public:
	EditPanel(int x, int y, int w, int h, const char* str = 0);
	~EditPanel();


	struct DrawInfo {
		Fl_RGB_Image* img = nullptr;
		Fl_Widget* ep = nullptr;
		cv::Mat* mat = nullptr;
	};

	void updateViewLabel();
	void startPusher(FramePusher* pusher);
	static void btn_clicked(Fl_Widget* widget, void* v);	//界面所有按钮的统一回调触发函数
	static void drawImg(cv::Mat mat, void* val);
protected:
	void addImgSource(std::shared_ptr<ImgSource> imgSource);
	void removeSource(int index);
	void resetMoveLabel(int index);
private:
	Fl_Box* m_frameShow = nullptr;			//帧画板
	//操作图源在画板的位置
	int m_moveOffset = 1;					//按下一次修改位置的步进长度
	Fl_Button* m_up = nullptr;				//上移图源位置
	void btn_up_do();
	Fl_Button* m_down = nullptr;			//下移图源位置
	void btn_down_do();
	Fl_Button* m_left = nullptr;			//左移图源位置
	void btn_left_do();
	Fl_Button* m_right = nullptr;			//右移图源位置
	void btn_right_do();
	Fl_Button* m_addLay = nullptr;			//新增图层
	void btn_addLay_do();
	Fl_Button* m_delLay = nullptr;			//删除图层
	void btn_delLay_do();
	Fl_Choice* m_layChoice = nullptr;		//选择图层
	void popmenu_do();
	Fl_Button* m_startPush = nullptr;		//开始推流
	void btn_startPush();
	Fl_Button* m_stopPush = nullptr;		//停止推流
	void btn_stopPush();

	cv::Mat m_showcvMat;					//cv帧缓存
	Fl_RGB_Image* m_showFrame = nullptr;	//显示帧缓存

	MoveLabel* m_moveLabel = nullptr;		//可移动的label，用来操作自主移动
	void moveLabel_do();
private:
	std::shared_ptr<FramePusher> m_framePusher = nullptr;	//帧推送器
	std::shared_ptr<FrameComposer> m_composer = nullptr;

	//VideoTimer m_composeTimer;
	int m_composeFrameFPS = 60;	//合成帧的fps，以该fps合成帧发送给推送器，以120帧合成以60帧合成

	cv::Mat m_bufferFrame;
	
	double m_fps = 30;	//
};