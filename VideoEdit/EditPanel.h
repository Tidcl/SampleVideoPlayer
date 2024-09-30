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
#include "MoveLabel.h"
#include "FramePusher.h"
#include "VideoPlay/PlayController.h"
#include "VideoPlay/VideoTimer.h"
#include "EditDecoder.h"

enum SourceType {
	Picture = 0,
	Gif,
	Video
};

class ImgSource{	//图源
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

class EditPanel : public Fl_Group {
public:
	EditPanel(int x, int y, int w, int h, const char* str);
	~EditPanel();

	void updateViewLabel();
	void startPusher(FramePusher* pusher);
	void updateAFrame();					//更新一个合成帧
	static void btn_clicked(Fl_Widget* widget, void* v);	//界面所有按钮的统一回调触发函数
protected:
	void addImgSource(std::shared_ptr<ImgSource> imgSource);
	void removeSource(int index);
	void resetMoveLabel(int index);
	void composeFrame();
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

	cv::Mat m_showcvMat;					//cv帧缓存
	Fl_RGB_Image* m_showFrame = nullptr;	//显示帧缓存

	MoveLabel* m_moveLabel = nullptr;		//可移动的label，用来操作自主移动
	void moveLabel_do();
private:
	FramePusher* m_framePusher = nullptr;	//帧推送器
	std::vector<std::shared_ptr<ImgSource>> m_imgSourceVec;	//图源列表

	VideoTimer m_composeTimer;

	cv::Mat m_bufferFrame;

	double m_fps = 30;
};