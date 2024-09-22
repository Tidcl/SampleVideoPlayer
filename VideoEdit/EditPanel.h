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
//#include <FL/Fl_Progress.H>
#include <FL/Fl_Choice.H>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "MoveLabel.h"

struct ImgSource{//图源
	std::string url;	//图源路径
	cv::Mat imgMat;	//图源矩阵，合成时使用
	//Fl_Image* frameImg = nullptr;	//
	int width = 0;	//长
	int height = 0;	//宽
	int x = 0;		//x位置
	int y = 0;		//y位置
};

class EditPanel : public Fl_Group {
public:
	EditPanel(int x, int y, int w, int h, const char* str);
	~EditPanel();

	void updateAFrame();	//更新一个合成帧
	static void btn_clicked(Fl_Widget* widget, void* v);	//界面所有按钮的统一回调触发函数
protected:
	void addImgSource(ImgSource imgSource);
	void removeSource(int index);
	void resetMoveLabel(int index);
private:
	Fl_Box* m_frameShow = nullptr;  //帧画板
	//操作图源在画板的位置
	int m_moveOffset = 1;	//按下一次修改位置的步进长度
	Fl_Button* m_up = nullptr; //上移图源位置
	void btn_up_do();
	Fl_Button* m_down = nullptr; //下移图源位置
	void btn_down_do();
	Fl_Button* m_left = nullptr; //左移图源位置
	void btn_left_do();
	Fl_Button* m_right = nullptr; //右移图源位置
	void btn_right_do();
	Fl_Button* m_addLay = nullptr;	//新增图层
	void btn_addLay_do();
	Fl_Button* m_delLay = nullptr;	//删除图层
	void btn_delLay_do();
	//选择控制的图层
	Fl_Choice* m_layChoice = nullptr;	//
	void popmenu_do();

	cv::Mat m_showcvMat;		//cv帧缓存
	Fl_RGB_Image* m_showFrame = nullptr;	//组件显示用帧

	std::vector<ImgSource> m_imgSourceVec;//图源列表

	MoveLabel* m_moveLabel = nullptr; //可移动的label
	void label_do();
};