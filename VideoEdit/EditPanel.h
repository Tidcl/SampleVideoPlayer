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

class ImgSource{	//ͼԴ
public:
	ImgSource();
	~ImgSource();

	cv::Mat& getMat();

	cv::Mat& getPointTimeMat(double time);

	void initSource();

	std::string m_url;//ͼԴ·��
	cv::Mat m_imgMat;	//ͼԴ���󣬺ϳ�ʱʹ��
	int m_width = 0;	//��
	int m_height = 0;	//��
	int m_x = 0;		//xλ��
	int m_y = 0;		//yλ��
	int m_fps = 0;	

	double m_startPlayTime;	//��ʼ����ʱ��
	double m_pauseTime;	//��ͣ��ʱ��

	SourceType m_type;
	bool m_pauseFlag = false;
	cv::Mat m_lastVideoMat;
	//����Decoder��֡
	std::shared_ptr<EditDecoder> m_decoder = nullptr;
};

class EditPanel : public Fl_Group {
public:
	EditPanel(int x, int y, int w, int h, const char* str);
	~EditPanel();

	void updateViewLabel();
	void startPusher(FramePusher* pusher);
	void updateAFrame();					//����һ���ϳ�֡
	static void btn_clicked(Fl_Widget* widget, void* v);	//�������а�ť��ͳһ�ص���������
protected:
	void addImgSource(std::shared_ptr<ImgSource> imgSource);
	void removeSource(int index);
	void resetMoveLabel(int index);
	void composeFrame();
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

	cv::Mat m_showcvMat;					//cv֡����
	Fl_RGB_Image* m_showFrame = nullptr;	//��ʾ֡����

	MoveLabel* m_moveLabel = nullptr;		//���ƶ���label���������������ƶ�
	void moveLabel_do();
private:
	FramePusher* m_framePusher = nullptr;	//֡������
	std::vector<std::shared_ptr<ImgSource>> m_imgSourceVec;	//ͼԴ�б�

	VideoTimer m_composeTimer;

	cv::Mat m_bufferFrame;

	double m_fps = 30;
};