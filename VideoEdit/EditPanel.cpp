#include <chrono>
#include <FL/Fl_File_Chooser.H>
#include "EditPanel.h"
#include <opencv2/opencv.hpp>
EditPanel::EditPanel(int x, int y, int w, int h, const char* str)
	:Fl_Group(x,y,w,h, str)
{
	m_frameShow = new Fl_Box(100, 10, w - 100, h - 50, 0);	

	int contolerYOffset = 10;
	m_up = new Fl_Button(30, 0 + contolerYOffset, 30, 20, "up");
	m_down = new Fl_Button(30, 20 + contolerYOffset, 30, 20, "down");
	m_left = new Fl_Button(0, 0 + contolerYOffset, 30, 40, "left");
	m_right = new Fl_Button(60, 0 + contolerYOffset, 30, 40, "right");

	m_layChoice = new Fl_Choice(0, 50, 90, 20, 0);

	m_addLay = new Fl_Button(0, 80, 45, 30, "add lay");
	m_delLay = new Fl_Button(50, 80, 45, 30, "del lay");

	m_moveLabel = new MoveLabel(100, 10, 150, 50, nullptr);

	m_startPush = new Fl_Button(5, 130, 90, 30, "start push");
	m_stopPush = new Fl_Button(5, 165, 90, 30, "stop push");

	m_up->callback(&EditPanel::btn_clicked, this);
	m_down->callback(&EditPanel::btn_clicked, this);
	m_left->callback(&EditPanel::btn_clicked, this);
	m_right->callback(&EditPanel::btn_clicked, this);
	m_addLay->callback(&EditPanel::btn_clicked, this);
	m_delLay->callback(&EditPanel::btn_clicked, this);
	m_layChoice->callback(&EditPanel::btn_clicked, this);
	m_moveLabel->callback(&EditPanel::btn_clicked, this);
	m_startPush->callback(&EditPanel::btn_clicked, this);
	m_stopPush->callback(&EditPanel::btn_clicked, this);


	std::shared_ptr<ImgSource> imgSource1 = std::make_shared<ImgSource>();
	imgSource1->m_url = "../../Resource/2.jpg";
	imgSource1->m_type = SourceType::Picture;
	imgSource1->m_x = 0;
	imgSource1->m_y = 0;
	imgSource1->m_width = m_frameShow->w();
	imgSource1->m_height = m_frameShow->h();
	imgSource1->initSource();

	std::shared_ptr<ImgSource>  imgSource2 = std::make_shared<ImgSource>();
	imgSource2->m_url = "../../Resource/SampleVideo_1280x720_10mb.mp4";	//C:/Users/xctan/Videos/SampleVideo_1280x720_10mb.mp4 C:/Users/xctan/Pictures/gif5.gif
	imgSource2->m_type = SourceType::Video;
	imgSource2->m_imgMat = cv::imread(imgSource2->m_url, cv::IMREAD_COLOR);
	imgSource2->m_pauseFlag = false;
	//imgSource2->m_x = 250;
	//imgSource2->m_y = 0;
	//imgSource2->m_width = 160 * 3;
	//imgSource2->m_height = 90 * 3;
	imgSource2->m_x = 0;
	imgSource2->m_y = 0;
	imgSource2->m_width = imgSource1->m_width;
	imgSource2->m_height = imgSource1->m_height;
	imgSource2->initSource();

	std::shared_ptr<ImgSource>  imgSource3 = std::make_shared<ImgSource>();
	imgSource3->m_url = "../../Resource/gif5.gif";	//C:/Users/xctan/Pictures/5.jpg
	imgSource3->m_type = SourceType::Gif;
	imgSource3->m_x = 0;
	imgSource3->m_y = 0;
	imgSource3->m_width = 200;
	imgSource3->m_height = 200;
	imgSource3->initSource();


	m_framePusher = std::make_shared<FramePusher>();
	m_composer = std::make_shared<FrameComposer>();
	m_composer->setPusher(m_framePusher);
	m_composer->setShowCallback(drawImg, m_frameShow);
	m_composer->setSize(m_frameShow->w(), m_frameShow->h());
	m_composer->startCompose();

	addImgSource(imgSource1);
	addImgSource(imgSource2);
	addImgSource(imgSource3);
	resetMoveLabel(2);

}

EditPanel::~EditPanel()
{
	//m_composeTimer.stop();

	m_composer->stopCompose();

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

	delete m_moveLabel;
	m_moveLabel = nullptr;

	delete m_showFrame;
	m_showFrame = nullptr;
}

void EditPanel::updateViewLabel()
{
	//Fl_RGB_Image imgFrame(m_showFrame);
	Fl_Image* copyFrame = m_showFrame->copy();
	m_frameShow->image(copyFrame);
	this->redraw();
	delete copyFrame;
}

void EditPanel::startPusher(FramePusher* pusher)
{
	pusher->openCodec(m_frameShow->w(), m_frameShow->h(), m_composeFrameFPS);
	//pusher->setPushFPS(m_composeFrameFPS);
	pusher->startPush();
	//m_framePusher = pusher;
	//updateAFrame();
}

void EditPanel::updateShowMat(cv::Mat mat)
{
	m_bufferFrame = mat;
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
	else if (widget == (Fl_Button*)ep->m_startPush)
	{
		ep->btn_startPush();
	}
	else if (widget == (Fl_Button*)ep->m_stopPush)
	{
		ep->btn_stopPush();
	}
}


void EditPanel::addImgSource(std::shared_ptr<ImgSource> imgSource)
{
	m_composer->addImgSource(imgSource);
	char intStr[8] = {0};
	itoa(m_composer->imgSourceCount(), intStr, 10);
	

	m_layChoice->add(intStr);
	m_layChoice->value(m_composer->imgSourceCount() - 1);
}

void EditPanel::removeSource(int index)
{
	if (index == -1) return;

	m_layChoice->clear();
	m_composer->removeImgSource(index);
	char intStr[8] = {0};
	for (size_t i = 0; i < m_composer->imgSourceCount(); i++)
	{
		memset(intStr, 0, 8);
		itoa(i + 1, intStr, 10);
		m_layChoice->add(intStr);
	}
}

void EditPanel::resetMoveLabel(int index)
{
	if (index == -1) return;
	auto imgSource = m_composer->imgSource(index);
	m_moveLabel->resize(m_frameShow->x() + (imgSource)->m_x, m_frameShow->y() + (imgSource)->m_y, (imgSource)->m_width, (imgSource)->m_height);
	redraw();
}

void EditPanel::drawImg(cv::Mat mat, void* val)
{
	if (mat.empty()) return;

	DrawInfo* di = new DrawInfo();
	di->mat = new cv::Mat;
	cv::cvtColor(mat, *(di->mat), cv::COLOR_BGR2RGB);
	di->ep = (Fl_Widget*)val;
	Fl::awake([](void* val) {
		static DrawInfo* sdi = nullptr;
		if (sdi)
		{
			delete sdi->mat;
			delete sdi->img;
			delete sdi;
			sdi = nullptr;
		}

		DrawInfo* di = (DrawInfo*)val;
		di->img = new Fl_RGB_Image(di->mat->data,	//��di->img���ݺܿ�������rgbImgMat���������ڱ������ָ��
			di->mat->cols,
			di->mat->rows,
			3,
			di->mat->step);
		di->ep->image(di->img);
		di->ep->redraw();
		sdi = di;
	}, di);//�˴�ֱ�����һ������������Ҳָ������label��image���
	//Fl::flush();
	Fl::check();
}

void EditPanel::btn_up_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_composer->imgSource(choiceValue);
	int yOffset = (element)->m_height - m_frameShow->h();
	if (yOffset < 0 && ((element)->m_y - m_moveOffset) >= 1) (element)->m_y -= m_moveOffset;

}

void EditPanel::btn_down_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_composer->imgSource(choiceValue);
	int yOffset = (element)->m_height - m_frameShow->h();
	if (yOffset < 0 && ((element)->m_y + (element)->m_height + m_moveOffset) <= m_frameShow->h()) (element)->m_y += m_moveOffset;

}

void EditPanel::btn_left_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_composer->imgSource(choiceValue);
	int xOffset = (element)->m_width - m_frameShow->w();
	if(xOffset < 0 && ((element)->m_x - m_moveOffset) >= 1) (element)->m_x -= m_moveOffset;

}

void EditPanel::btn_right_do()
{
	auto choiceValue = m_layChoice->value();
	if (choiceValue == -1) return;
	auto element = m_composer->imgSource(choiceValue);
	int xOffset = (element)->m_width - m_frameShow->w();
	if (xOffset < 0 && ((element)->m_x + (element)->m_width + m_moveOffset) <= m_frameShow->w()) (element)->m_x += m_moveOffset;

}

void EditPanel::btn_addLay_do()
{
	//���ļ�ѡ����
	Fl_File_Chooser* fc = new Fl_File_Chooser(".", "Image Files (*.jpg, *.jpeg, *.png, *.gif)", Fl_File_Chooser::SINGLE, "Select a img file");
	fc->callback([](Fl_File_Chooser* fChoice, void* source) {
		if(fChoice->count() <= 0) return;

		EditPanel* ep = (EditPanel*)source;
		std::shared_ptr<ImgSource> imgSource1 = std::make_shared<ImgSource>();
		imgSource1->m_url = fChoice->value(0);
		imgSource1->m_imgMat = cv::imread(imgSource1->m_url, cv::IMREAD_COLOR);
		if (imgSource1->m_imgMat.rows == 0 || imgSource1->m_imgMat.cols == 0) return;
		imgSource1->m_x = 0;
		imgSource1->m_y = 0;
		imgSource1->m_width = ep->m_frameShow->w();
		imgSource1->m_height = ep->m_frameShow->h();
		imgSource1->initSource();
		ep->addImgSource(imgSource1);
	}, this);
	fc->show();

}

void EditPanel::btn_delLay_do()
{
	removeSource(m_layChoice->value());
}

void EditPanel::popmenu_do()
{
	resetMoveLabel(m_layChoice->value());
}

void EditPanel::btn_startPush()
{
	m_framePusher->stopPush();
	std::thread t([this]() {
		if (m_framePusher->openCodec(m_frameShow->w(), m_frameShow->h(), m_composeFrameFPS) == 0)
			m_framePusher->startPush();
	});
	t.detach();
}

void EditPanel::btn_stopPush()
{

	m_framePusher->stopPush();
}

void EditPanel::moveLabel_do()
{
	int index = m_layChoice->value();
	if (index == -1) return;
	auto imgSource = m_composer->imgSource(index);
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
