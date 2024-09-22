#include <FL/Fl_File_Chooser.H>
#include "EditPanel.h"

EditPanel::EditPanel(int x, int y, int w, int h, const char* str)
	:Fl_Group(x,y,w,h, str)
{
	m_frameShow = new Fl_Box(100, 30, w - 100, h - 50, "show img");

	int contolerYOffset = 5;
	m_up = new Fl_Button(30, 0 + contolerYOffset, 30, 20, "up");
	m_down = new Fl_Button(30, 20 + contolerYOffset, 30, 20, "down");
	m_left = new Fl_Button(0, 0 + contolerYOffset, 30, 40, "left");
	m_right = new Fl_Button(60, 0 + contolerYOffset, 30, 40, "right");

	m_layChoice = new Fl_Choice(0, 50, 90, 20);

	m_addLay = new Fl_Button(0, 80, 45, 30, "add lay");
	m_delLay = new Fl_Button(50, 80, 45, 30, "del lay");

	m_moveLabel = new MoveLabel(100, 30, 30, 30, "X");

	m_up->callback(&EditPanel::btn_clicked, this);
	m_down->callback(&EditPanel::btn_clicked, this);
	m_left->callback(&EditPanel::btn_clicked, this);
	m_right->callback(&EditPanel::btn_clicked, this);
	m_addLay->callback(&EditPanel::btn_clicked, this);
	m_delLay->callback(&EditPanel::btn_clicked, this);
	m_layChoice->callback(&EditPanel::btn_clicked, this);
	m_moveLabel->callback(&EditPanel::btn_clicked, this);

	m_imgSourceVec;
	ImgSource imgSource1;
	imgSource1.url = "C:/Users/xctan/Pictures/2.jpg";
	imgSource1.imgMat = cv::imread(imgSource1.url, cv::IMREAD_COLOR);
	imgSource1.x = 0;
	imgSource1.y = 0;
	imgSource1.width = w - 100;
	imgSource1.height = h - 50;
	ImgSource imgSource2;
	imgSource2.url = "C:/Users/xctan/Pictures/4.jpg";
	imgSource2.imgMat = cv::imread(imgSource2.url, cv::IMREAD_COLOR);
	imgSource2.x = 0;
	imgSource2.y = 0;
	imgSource2.width = 50;
	imgSource2.height = 50;
	ImgSource imgSource3;
	imgSource3.url = "C:/Users/xctan/Pictures/5.jpg";
	imgSource3.imgMat = cv::imread(imgSource3.url, cv::IMREAD_COLOR);
	imgSource3.x = 0;
	imgSource3.y = 50;
	imgSource3.width = 150;
	imgSource3.height = 50;
	addImgSource(imgSource1);
	addImgSource(imgSource2);
	addImgSource(imgSource3);

	updateAFrame();
}

EditPanel::~EditPanel()
{
	delete m_frameShow;
	m_frameShow = nullptr;

	delete m_up;
	m_up = nullptr;
	delete m_down;
	m_down = nullptr;
	delete m_left;
	m_left = nullptr;
	delete m_right;
	m_right = nullptr;
	delete m_layChoice;
	m_layChoice = nullptr;

	delete m_addLay;
	m_addLay = nullptr;
	delete m_delLay;
	m_delLay = nullptr;
}

void EditPanel::updateAFrame()
{
	//创建合成帧缓冲
	cv::Mat bufferFrame = cv::Mat::zeros(cv::Size(m_frameShow->w(), m_frameShow->h()), CV_8UC3);
	//cv::Mat bufferFrame(m_frameShow->w(), m_frameShow->h(), CV_8UC3, cv::Scalar(0, 0, 0));
	bufferFrame.setTo(cv::Scalar(255, 255, 255));

	//便利图源合成到合成帧中
	for (ImgSource& imgSour : m_imgSourceVec) 
	{
		int x = imgSour.x;
		int y = imgSour.y;
		int width = imgSour.width;
		int height = imgSour.height;

		//将图源重新缩放
		//cv::Mat resizeImgMat = bufferFrame(cv::Rect(x, y, width, height));
		cv::Mat resizeImgMat;
		cv::resize(imgSour.imgMat, resizeImgMat, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);
		//如果焦点区域超出就需要裁剪原图
		resizeImgMat.copyTo(bufferFrame(cv::Rect(x, y, width, height)));
		//imgMat.copyTo(resizeImgMat);
	}

	//合成帧更新到界面
	cv::Mat rgbImgMat;
	cv::cvtColor(bufferFrame, rgbImgMat, cv::COLOR_BGR2RGB);
	m_showcvMat = rgbImgMat;	//显示合成帧存到成员变量

	m_showFrame = new Fl_RGB_Image(rgbImgMat.data,	//该数据很可能随着rgbImgMat的生命周期变成悬挂指针
		rgbImgMat.cols,
		rgbImgMat.rows,
		3,
		rgbImgMat.step);
	

	Fl_Image* cimg = m_showFrame->copy(m_frameShow->w(), m_frameShow->h());
	m_frameShow->image(cimg);
	//m_frameShow->redraw();
	redraw();
}

void EditPanel::btn_clicked(Fl_Widget* widget, void* v)
{
	EditPanel* ep = (EditPanel*)v;

	if (widget == (Fl_Button*)ep->m_up)
	{
		ep->btn_up_do();
	}
	else if (widget == (Fl_Button*)ep->m_down)
	{
		ep->btn_down_do();
	}
	else if (widget == (Fl_Button*)ep->m_left)
	{
		ep->btn_left_do();
	}
	else if (widget == (Fl_Button*)ep->m_right)
	{
		ep->btn_right_do();
	}
	else if (widget == (Fl_Button*)ep->m_addLay)
	{
		ep->btn_addLay_do();
	}
	else if (widget == (Fl_Button*)ep->m_delLay)
	{
		ep->btn_delLay_do();
	}
	else if (widget == (Fl_Button*)ep->m_layChoice)
	{
		ep->popmenu_do();
	}
	else if (widget == (Fl_Button*)ep->m_moveLabel)
	{
		ep->label_do();
	}
}

void EditPanel::addImgSource(ImgSource imgSource)
{
	m_imgSourceVec.push_back(imgSource); 
	char intStr[8] = {0};
	itoa(m_imgSourceVec.size(), intStr, 10);
	

	m_layChoice->add(intStr);
	m_layChoice->value(m_imgSourceVec.size() - 1);
	resetMoveLabel(m_imgSourceVec.size() - 1);
}

void EditPanel::removeSource(int index)
{
	if (index == -1) return;

	m_layChoice->clear();
	m_imgSourceVec.erase(m_imgSourceVec.begin() + index);
	char intStr[8] = {0};
	for (size_t i = 0; i < m_imgSourceVec.size(); i++)
	{
		memset(intStr, 0, 8);
		itoa(i + 1, intStr, 10);
		m_layChoice->add(intStr);
	}
	updateAFrame();
}

void EditPanel::resetMoveLabel(int index)
{
	if (index == -1) return;
	auto& imgSource = m_imgSourceVec.begin() + index;
	m_moveLabel->resize(m_frameShow->x() + imgSource->x, m_frameShow->y() + imgSource->y, imgSource->width, imgSource->height);
	//m_moveLabel->position(m_frameShow->x() + imgSource->x, m_frameShow->y() + imgSource->y);
	redraw();
}

void EditPanel::btn_up_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int yOffset = element->height - m_frameShow->h();
	if (yOffset < 0 && (element->y - m_moveOffset) >= 1) element->y -= m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_down_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int yOffset = element->height - m_frameShow->h();
	if (yOffset < 0 && (element->y + element->height + m_moveOffset) <= m_frameShow->h()) element->y += m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_left_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int xOffset = element->width - m_frameShow->w();
	if(xOffset < 0 && (element->x - m_moveOffset) >= 1) element->x -= m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_right_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int xOffset = element->width - m_frameShow->w();
	if (xOffset < 0 && (element->x + element->width + m_moveOffset) <= m_frameShow->w()) element->x += m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_addLay_do()
{
	//打开文件选择器
	Fl_File_Chooser* fc = new Fl_File_Chooser(".", "Image Files (*.jpg, *.jpeg, *.png, *.gif)", Fl_File_Chooser::SINGLE, "Select a img file");
	fc->callback([](Fl_File_Chooser* fChoice, void* source) {
		if(fChoice->count() <= 0) return;

		//ImgSource* is = (ImgSource*)source;
		EditPanel* ep = (EditPanel*)source;
		ImgSource imgSource1;
		imgSource1.url = fChoice->value(0);
		imgSource1.imgMat = cv::imread(imgSource1.url, cv::IMREAD_COLOR);
		if (imgSource1.imgMat.rows == 0 || imgSource1.imgMat.cols == 0) return;
		imgSource1.x = 0;
		imgSource1.y = 0;
		imgSource1.width = ep->m_frameShow->w();
		imgSource1.height = ep->m_frameShow->h();
		ep->addImgSource(imgSource1);
		ep->updateAFrame();
	}, this);
	fc->show();

	//imgSource1.url = "C:/Users/xctan/Pictures/2.jpg";

}

void EditPanel::btn_delLay_do()
{
	removeSource(m_layChoice->value());
}

void EditPanel::popmenu_do()
{
	resetMoveLabel(m_layChoice->value());
}

void EditPanel::label_do()
{
	//std::cout << "move done" << std::endl;
	int index = m_layChoice->value();
	if (index == -1) return;
	auto& imgSource = m_imgSourceVec.begin() + index;
	int offset_x = m_moveLabel->x() - m_frameShow->x();
	int offset_y = m_moveLabel->y() - m_frameShow->y();

	if (offset_x + imgSource->width > m_frameShow->w()
		|| offset_y + imgSource->height > m_frameShow->h()
		|| offset_x < 0 || offset_y < 0)
	{
		resetMoveLabel(index);
		return;
	}
	imgSource->x = offset_x;
	imgSource->y = offset_y;

	updateAFrame();
}
