#include "PlayWidget.h"
#include <thread>
#include <chrono>

Fl_RGB_Image* AVFrameToFlRGBImage(AVFrame* frame, void*& dataAddr) {
	if (frame == nullptr) return nullptr;


	int width = frame->width;
	int height = frame->height;

	// 创建SwsContext，用于颜色空间转换
	SwsContext* sws_ctx = sws_getContext(
		width, height, (AVPixelFormat)frame->format,  // 源格式
		width, height, AV_PIX_FMT_RGB24,    // 目标格式
		SWS_BILINEAR, nullptr, nullptr, nullptr
	);

	// 将YUV420P转换为RGB24
	uint8_t* rgb_data[1] = { new uint8_t[frame->width * frame->height * 3 + 32] };
	int rgb_linesize[1] = { frame->width * 3 };
	sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, rgb_data, rgb_linesize);

	Fl_RGB_Image* img = new Fl_RGB_Image(rgb_data[0], frame->width, frame->height, 3);
	dataAddr = rgb_data[0];
	// 清理资源
	sws_freeContext(sws_ctx);

	return img;
}


struct UpdateLabelInfo {
	UpdateLabelInfo() = default;
	~UpdateLabelInfo() = default;
	Fl_Box* _label;
	Fl_RGB_Image* _img;
	void* _imgData;	//指向图像数据的指针，用来释放图像内存

	Fl_Progress* _progress;
	double _progressValue;
	double _totalProgressValue;
};
void updateLabel(void* _info)
{
	static Fl_Image* simg = nullptr;
	if (simg) 
	{ 
		delete simg;
		simg = nullptr; 
	}

	UpdateLabelInfo* info = (UpdateLabelInfo*)_info;
	Fl_Box* label = info->_label;
	if (info->_img == nullptr)
	{
		label->image(nullptr);
		label->redraw();
	}
	else
	{
		Fl_Image* cimg = info->_img->copy(label->w(), label->h());
		label->image(cimg);
		label->redraw();
		//delete[] info->_img->data()[0];
		
		delete info->_img;
		delete[] info->_imgData;
		simg = cimg;


		info->_progress->minimum((float)0);
		info->_progress->maximum((float)info->_totalProgressValue);
		info->_progress->value((float)info->_progressValue);
		info->_progress->redraw();
	}

	delete info;
};




PlayWidget::PlayWidget(int x, int y, int w, int h, char* str)
	:Fl_Group(x, y, w, h, str)
{
	//界面位置
	m_input = new Fl_Input(x, 5, w, 20, "");
	m_btn = new Fl_Button(x + 30, this->y() + 30, (int)(w * 0.4), 20, "播放");
	m_pauseBtn = new Fl_Button(x + 30 + (int)(w*0.5), this->y() + 30, (int)(w*0.4), 20, "暂停");
	m_label = new Fl_Box(x, m_btn->y() + m_btn->h() + 10, w, w/16*9, "");
	m_progress = new ClickProgress(x, m_label->y() + m_label->h(), w - 70, 20);
	m_choice = new Fl_Choice(x + m_progress->w() + 10, m_label->y() + m_label->h(), 60, 20);
	m_choice->add("x0.25");
	m_choice->add("x0.5");
	m_choice->add("x1");
	m_choice->add("x2");
	m_choice->add("x5");
	m_choice->value(2);

	//回调设置
	m_btn->callback(PlayWidget::btn_clicked, this);
	m_pauseBtn->callback(PlayWidget::btn_clicked, this);
	m_choice->callback(&PlayWidget::btn_clicked, this);
	m_progress->setCallBackFunc(&PlayWidget::btn_clicked, this);


	m_pc = std::make_shared<PlayController>();
	m_pc->setFrameRecall([](void* f, void* thi, double playTimeMS) {
		void* imgDataPtr = nullptr;
		PlayWidget* thii = (PlayWidget*)thi;
		Fl_RGB_Image* img1 = AVFrameToFlRGBImage((AVFrame*)f, imgDataPtr); //得到新的pix
		UpdateLabelInfo* info = new UpdateLabelInfo();
		info->_label =  thii->m_label;
		info->_img = img1;
		info->_imgData = imgDataPtr;	

		info->_progress = thii->m_progress;
		info->_progressValue = playTimeMS * 0.001;
		info->_totalProgressValue = thii->m_pc->totalDuration();
		Fl::awake(updateLabel, info);//此处直接设计一个函数，参数也指定传入label和image深拷贝
		Fl::check();
	}, this);

	this->end();
};

PlayWidget::~PlayWidget()
{
	delete m_label; m_label = nullptr;
	delete m_btn; m_btn = nullptr;
	delete m_pauseBtn;
	delete m_progress;
	delete m_choice;
}


void PlayWidget::btn_clicked(Fl_Widget* widget, void* v)
{
	PlayWidget* pw = (PlayWidget*)v;

	if (widget == (Fl_Widget*)pw->m_btn)
	{
		pw->btn_play_do();
	}
	else if(widget == pw->m_pauseBtn)
	{
		pw->btn_pause_do();
	}
	else if (widget == pw->m_choice)
	{
		pw->choice_playSpeed();
	}
	else if (widget == pw->m_progress)
	{
		pw->progress_play();
	}
}

void PlayWidget::btn_pause_do()
{
	if (m_pc->status() == PlayStatus::stop)
	{
		return;
	}
	m_pc->setPause(!m_pc->pause());
}

void PlayWidget::btn_play_do()
{
	std::shared_ptr<PlayController> pc = m_pc;

	if (pc->status() == PlayStatus::stop)
	{
		pc->setVideoUrl(m_input->value());
		pc->startPlay();
	}
	else
	{
		pc->stopPlay();
		pc->setVideoUrl(m_input->value());
		pc->startPlay();
	}
}

void PlayWidget::choice_playSpeed()
{
	const char* str = m_choice->text();
	double playSpeed = std::stod(str + 1);
	m_pc->setPlaySpeed(playSpeed);
}

void PlayWidget::progress_play()
{
	double progressValue = m_progress->value();
	m_pc->setSeekTime((long)progressValue * 1000);
}
