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
	static void btn_clicked(Fl_Widget* widget, void* v);	//�������а�ť��ͳһ�ص���������
	static void drawImg(cv::Mat mat, void* val);
protected:
	void addImgSource(std::shared_ptr<ImgSource> imgSource);
	void removeSource(int index);
	void resetMoveLabel(int index);
private:
	Fl_Box* m_frameShow = nullptr;			//֡����
	//����ͼԴ�ڻ����λ��
	int m_moveOffset = 1;					//����һ���޸�λ�õĲ�������
	Fl_Button* m_up = nullptr;				//����ͼԴλ��
	void btn_up_do();
	Fl_Button* m_down = nullptr;			//����ͼԴλ��
	void btn_down_do();
	Fl_Button* m_left = nullptr;			//����ͼԴλ��
	void btn_left_do();
	Fl_Button* m_right = nullptr;			//����ͼԴλ��
	void btn_right_do();
	Fl_Button* m_addLay = nullptr;			//����ͼ��
	void btn_addLay_do();
	Fl_Button* m_delLay = nullptr;			//ɾ��ͼ��
	void btn_delLay_do();
	Fl_Choice* m_layChoice = nullptr;		//ѡ��ͼ��
	void popmenu_do();
	Fl_Button* m_startPush = nullptr;		//��ʼ����
	void btn_startPush();
	Fl_Button* m_stopPush = nullptr;		//ֹͣ����
	void btn_stopPush();

	cv::Mat m_showcvMat;					//cv֡����
	Fl_RGB_Image* m_showFrame = nullptr;	//��ʾ֡����

	MoveLabel* m_moveLabel = nullptr;		//���ƶ���label���������������ƶ�
	void moveLabel_do();
private:
	std::shared_ptr<FramePusher> m_framePusher = nullptr;	//֡������
	std::shared_ptr<FrameComposer> m_composer = nullptr;

	//VideoTimer m_composeTimer;
	int m_composeFrameFPS = 60;	//�ϳ�֡��fps���Ը�fps�ϳ�֡���͸�����������120֡�ϳ���60֡�ϳ�

	cv::Mat m_bufferFrame;
	
	double m_fps = 30;	//
};