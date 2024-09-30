#include <FL/Fl_File_Chooser.H>
#include "EditPanel.h"
#include <opencv2/opencv.hpp>
EditPanel::EditPanel(int x, int y, int w, int h, const char* str)
	:Fl_Group(x,y,w,h, str)
{
	m_frameShow = new Fl_Box(100, 10, w - 100, h - 50);	

	int contolerYOffset = 10;
	m_up = new Fl_Button(30, 0 + contolerYOffset, 30, 20, "up");
	m_down = new Fl_Button(30, 20 + contolerYOffset, 30, 20, "down");
	m_left = new Fl_Button(0, 0 + contolerYOffset, 30, 40, "left");
	m_right = new Fl_Button(60, 0 + contolerYOffset, 30, 40, "right");

	m_layChoice = new Fl_Choice(0, 50, 90, 20);

	m_addLay = new Fl_Button(0, 80, 45, 30, "add lay");
	m_delLay = new Fl_Button(50, 80, 45, 30, "del lay");

	m_moveLabel = new MoveLabel(100, 10, 150, 50, "");

	m_up->callback(&EditPanel::btn_clicked, this);
	m_down->callback(&EditPanel::btn_clicked, this);
	m_left->callback(&EditPanel::btn_clicked, this);
	m_right->callback(&EditPanel::btn_clicked, this);
	m_addLay->callback(&EditPanel::btn_clicked, this);
	m_delLay->callback(&EditPanel::btn_clicked, this);
	m_layChoice->callback(&EditPanel::btn_clicked, this);
	m_moveLabel->callback(&EditPanel::btn_clicked, this);

	//m_bufferFrame = cv::Mat::zeros(cv::Size(m_frameShow->w(), m_frameShow->h()), CV_8UC3);

	m_imgSourceVec;
	std::shared_ptr<ImgSource> imgSource1 = std::make_shared<ImgSource>();
	imgSource1->m_url = "C:/Users/xctan/Pictures/2.jpg";
	imgSource1->m_type = SourceType::Picture;
	imgSource1->m_x = 0;
	imgSource1->m_y = 0;
	imgSource1->m_width = m_frameShow->w();
	imgSource1->m_height = m_frameShow->h();
	imgSource1->initSource();

	std::shared_ptr<ImgSource>  imgSource2 = std::make_shared<ImgSource>();
	imgSource2->m_url = "C:/Users/xctan/Videos/SampleVideo_1280x720_10mb.mp4";	//C:/Users/xctan/Videos/SampleVideo_1280x720_10mb.mp4 C:/Users/xctan/Pictures/gif5.gif
	imgSource2->m_type = SourceType::Video;
	imgSource2->m_imgMat = cv::imread(imgSource2->m_url, cv::IMREAD_COLOR);
	imgSource2->m_pauseFlag = false;
	imgSource2->m_x = 250;
	imgSource2->m_y = 0;
	imgSource2->m_width = 160 * 3;
	imgSource2->m_height = 90 * 3;
	imgSource2->initSource();

	std::shared_ptr<ImgSource>  imgSource3 = std::make_shared<ImgSource>();
	imgSource3->m_url = "C:/Users/xctan/Pictures/gif5.gif";	//C:/Users/xctan/Pictures/5.jpg
	imgSource3->m_type = SourceType::Gif;
	imgSource3->m_x = 0;
	imgSource3->m_y = 0;
	imgSource3->m_width = 200;
	imgSource3->m_height = 200;
	imgSource3->initSource();

	addImgSource(imgSource1);
	addImgSource(imgSource2);
	addImgSource(imgSource3);
	
	m_composeTimer.setInterval(1);
	m_composeTimer.setCallbackFun([](void* val) {
		EditPanel* ep = (EditPanel*)val;
		ep->composeFrame();
		//��֡������ȡ��
	}, this);
	m_composeTimer.start();
}

void EditPanel::composeFrame()
{
	auto startTime_t = std::chrono::high_resolution_clock::now();
	static double frameTime = 1000 / m_fps;
	static double m_playFrameMS = 0;
	static int bufferMaxCount = 10;
	static double frameIndex = 0;

	int nowTime = m_composeTimer.timeMS();

	if (m_framePusher && m_framePusher->bufferCount() >= 30)
	{
		return;
	}

	cv::Mat bufferFrame(m_frameShow->h(), m_frameShow->w(), CV_8UC3);
	//����ͼԴ�ϳɵ��ϳ�֡��
	for (auto imgSour : m_imgSourceVec)
	{
		cv::Mat& mat = imgSour->getPointTimeMat(m_playFrameMS);

		//��ͼԴ��������
		if (mat.rows == 0)
			continue;
		//����������򳬳�����Ҫ�ü�ԭͼ
		mat.copyTo(bufferFrame(cv::Rect(imgSour->m_x, imgSour->m_y, imgSour->m_width, imgSour->m_height)));
	}
	
	m_playFrameMS += frameTime;
	frameIndex++;
	if (m_framePusher)
	{
		m_framePusher->pushFrame(bufferFrame);
	}
}


EditPanel::~EditPanel()
{
	m_composeTimer.stop();
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

void EditPanel::updateViewLabel()
{
	m_frameShow->image(m_showFrame);
	this->redraw();
}

void EditPanel::startPusher(FramePusher* pusher)
{
	pusher->startPush();
	m_framePusher = pusher;
	//updateAFrame();
}

void EditPanel::updateAFrame()
{
	//����ͼԴ�ϳɵ��ϳ�֡��
	cv::Mat resizeImgMat;
	auto startTime = std::chrono::high_resolution_clock::now();
	
	for (auto imgSour : m_imgSourceVec) 
	{
		int x = imgSour->m_x;
		int y = imgSour->m_y;
		int width = imgSour->m_width;
		int height = imgSour->m_height;


		//��ͼԴ��������
		cv::Mat& mat = imgSour->getMat();
		if(mat.rows == 0) continue;
		//����������򳬳�����Ҫ�ü�ԭͼ
		mat.copyTo(m_bufferFrame(cv::Rect(x, y, width, height)));
	}

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

	//�ϳ�֡���µ�����
	cv::Mat rgbImgMat;
	cv::cvtColor(m_bufferFrame, rgbImgMat, cv::COLOR_BGR2RGB);
	//m_showcvMat = rgbImgMat;	//��ʾ�ϳ�֡�浽��Ա����
	if (m_showFrame) delete m_showFrame;	//������ʾ��Fl_RGB_Image
	m_showFrame = new Fl_RGB_Image(rgbImgMat.data,	//�����ݺܿ�������rgbImgMat���������ڱ������ָ��
		rgbImgMat.cols,
		rgbImgMat.rows,
		3,
		rgbImgMat.step);

	//cv::imshow("Image Window", m_bufferFrame);

	//m_frameShow->image(m_showFrame);
	//if(m_framePusher) m_framePusher->updateFrame(m_bufferFrame);

	//�ύ���߳�ˢ�£��������
	//Fl::awake([](void* val) {
	//	EditPanel* ep = (EditPanel*)val;
	//	ep->updateViewLabel();
	//}, this);//�˴�ֱ�����һ������������Ҳָ������label��image���
	//Fl::check();
	//redraw();
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
		ep->moveLabel_do();
	}
}

void EditPanel::addImgSource(std::shared_ptr<ImgSource> imgSource)
{
	m_imgSourceVec.push_back(imgSource); 
	char intStr[8] = {0};
	itoa(m_imgSourceVec.size(), intStr, 10);
	

	m_layChoice->add(intStr);
	m_layChoice->value(m_imgSourceVec.size() - 1);
	//resetMoveLabel(m_imgSourceVec.size() - 1);
}

void EditPanel::removeSource(int index)
{
	if (index == -1) return;

	m_layChoice->clear();
	auto ptr = (*(m_imgSourceVec.begin() + index))->m_decoder;
	if (ptr) { ptr->stopDecode(); }
	
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
	auto imgSource = m_imgSourceVec.begin() + index;
	m_moveLabel->resize(m_frameShow->x() + (*imgSource)->m_x, m_frameShow->y() + (*imgSource)->m_y, (*imgSource)->m_width, (*imgSource)->m_height);
	//m_moveLabel->position(m_frameShow->x() + imgSource->x, m_frameShow->y() + imgSource->y);
	redraw();
}



void EditPanel::btn_up_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int yOffset = (*element)->m_height - m_frameShow->h();
	if (yOffset < 0 && ((*element)->m_y - m_moveOffset) >= 1) (*element)->m_y -= m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_down_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int yOffset = (*element)->m_height - m_frameShow->h();
	if (yOffset < 0 && ((*element)->m_y + (*element)->m_height + m_moveOffset) <= m_frameShow->h()) (*element)->m_y += m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_left_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int xOffset = (*element)->m_width - m_frameShow->w();
	if(xOffset < 0 && ((*element)->m_x - m_moveOffset) >= 1) (*element)->m_x -= m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_right_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_imgSourceVec.begin() + choiceValue;
	int xOffset = (*element)->m_width - m_frameShow->w();
	if (xOffset < 0 && ((*element)->m_x + (*element)->m_width + m_moveOffset) <= m_frameShow->w()) (*element)->m_x += m_moveOffset;
	updateAFrame();
}

void EditPanel::btn_addLay_do()
{
	//���ļ�ѡ����
	Fl_File_Chooser* fc = new Fl_File_Chooser(".", "Image Files (*.jpg, *.jpeg, *.png, *.gif)", Fl_File_Chooser::SINGLE, "Select a img file");
	fc->callback([](Fl_File_Chooser* fChoice, void* source) {
		if(fChoice->count() <= 0) return;

		//ImgSource* is = (ImgSource*)source;
		EditPanel* ep = (EditPanel*)source;
		std::shared_ptr<ImgSource> imgSource1 = std::make_shared<ImgSource>();
		imgSource1->m_url = fChoice->value(0);
		imgSource1->m_imgMat = cv::imread(imgSource1->m_url, cv::IMREAD_COLOR);
		if (imgSource1->m_imgMat.rows == 0 || imgSource1->m_imgMat.cols == 0) return;
		imgSource1->m_x = 0;
		imgSource1->m_y = 0;
		imgSource1->m_width = ep->m_frameShow->w();
		imgSource1->m_height = ep->m_frameShow->h();
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

void EditPanel::moveLabel_do()
{
	//std::cout << "move done" << std::endl;
	int index = m_layChoice->value();
	if (index == -1) return;
	auto imgSourcet = m_imgSourceVec.begin() + index;
	std::shared_ptr<ImgSource> imgSource = *imgSourcet;
	int offset_x = m_moveLabel->x() - m_frameShow->x();
	int offset_y = m_moveLabel->y() - m_frameShow->y();

	if (offset_x + imgSource->m_width > m_frameShow->w()
		|| offset_y + imgSource->m_height > m_frameShow->h()
		|| offset_x < 0 || offset_y < 0)
	{
		resetMoveLabel(index);
		return;
	}
	imgSource->m_x = offset_x;
	imgSource->m_y = offset_y;

	//updateAFrame();
}

ImgSource::ImgSource()
{
}

ImgSource::~ImgSource()
{
}

cv::Mat& ImgSource::getMat()
{
	switch (m_type)
	{
	case Picture:
		return m_imgMat;
		break;
	case Gif:
	case Video:
		if (m_pauseFlag)
		{
			return m_lastVideoMat;
		}
		else
		{
			cv::Mat mat = m_decoder->popFrontMat();

			if (mat.rows == 0)
			{
				return m_lastVideoMat;
			}
			else
			{
				m_lastVideoMat = mat;
			}

			return m_lastVideoMat;
		}
		break;
	default:
		break;
	}
}

cv::Mat& ImgSource::getPointTimeMat(double time)
{
	switch (m_type)
	{
	case Picture:
		return m_imgMat;
		break;
	case Gif:
	case Video:
		if (m_pauseFlag)
		{
			return m_lastVideoMat;
		}
		else
		{
			double timeStep = m_decoder->frontTimeStep();
			while (timeStep < time && timeStep != -1)
			{
				cv::Mat mat = m_decoder->popFrontMat();
				if (mat.rows == 0)
				{
					return m_lastVideoMat;
				}
				else
				{
					std::swap(m_lastVideoMat, mat);
					timeStep = m_decoder->frontTimeStep();
				}
			}
			return m_lastVideoMat;
		}
		break;
	default:
		break;
	}
}

void ImgSource::initSource()
{
	switch (m_type)
	{
	case Picture:
	{
		cv::resize(cv::imread(m_url, cv::IMREAD_COLOR), m_imgMat, cv::Size(m_width, m_height));
	}
		break;
	case Gif:
	case Video:
	{
		m_decoder = std::make_shared<EditDecoder>();
		m_decoder->initDecode(m_url);
		m_fps = m_decoder->videoFps();
		m_decoder->m_width = m_width;
		m_decoder->m_height = m_height;
		m_decoder->startDecode();
	}
		break;
	default:
		break;
	}
}
