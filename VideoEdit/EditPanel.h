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

struct ImgSource{//ͼԴ
	std::string url;	//ͼԴ·��
	cv::Mat imgMat;	//ͼԴ���󣬺ϳ�ʱʹ��
	//Fl_Image* frameImg = nullptr;	//
	int width = 0;	//��
	int height = 0;	//��
	int x = 0;		//xλ��
	int y = 0;		//yλ��
};

class EditPanel : public Fl_Group {
public:
	EditPanel(int x, int y, int w, int h, const char* str);
	~EditPanel();

	void updateAFrame();	//����һ���ϳ�֡
	static void btn_clicked(Fl_Widget* widget, void* v);	//�������а�ť��ͳһ�ص���������
protected:
	void addImgSource(ImgSource imgSource);
	void removeSource(int index);
	void resetMoveLabel(int index);
private:
	Fl_Box* m_frameShow = nullptr;  //֡����
	//����ͼԴ�ڻ����λ��
	int m_moveOffset = 1;	//����һ���޸�λ�õĲ�������
	Fl_Button* m_up = nullptr; //����ͼԴλ��
	void btn_up_do();
	Fl_Button* m_down = nullptr; //����ͼԴλ��
	void btn_down_do();
	Fl_Button* m_left = nullptr; //����ͼԴλ��
	void btn_left_do();
	Fl_Button* m_right = nullptr; //����ͼԴλ��
	void btn_right_do();
	Fl_Button* m_addLay = nullptr;	//����ͼ��
	void btn_addLay_do();
	Fl_Button* m_delLay = nullptr;	//ɾ��ͼ��
	void btn_delLay_do();
	//ѡ����Ƶ�ͼ��
	Fl_Choice* m_layChoice = nullptr;	//
	void popmenu_do();

	cv::Mat m_showcvMat;		//cv֡����
	Fl_RGB_Image* m_showFrame = nullptr;	//�����ʾ��֡

	std::vector<ImgSource> m_imgSourceVec;//ͼԴ�б�

	MoveLabel* m_moveLabel = nullptr; //���ƶ���label
	void label_do();
};