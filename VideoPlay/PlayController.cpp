#include "PlayController.h"
#include "VideoDecoder.h"
#include "VideoEdit/EditDecoder.h"

void PlayController::seek(long timeMS)
{
	m_playStart = std::chrono::high_resolution_clock::now();
	m_seekTimeMs = timeMS;
	m_pauseMS = 0;
	m_playStartTime = 0;
	//if(m_playDecoder) m_playDecoder->videoSeek(m_seekTimeMs);
}

void PlayController::setSeekTime(long timeMS)
{
	m_seekFlag = true;
	m_seekTimeMs = timeMS;
}

double PlayController::totalDuration()
{
	return m_totalDuration;
}

void PlayController::setPlaySpeed(double playSpeed)
{
	m_playSpeed = playSpeed;
	setSeekTime((long)playTimeSeconds());
}

void PlayController::setFrameRecall(frameRecall* func, void* v)
{
	m_func = func;
	m_v = v;
};

//void PlayController::updatePlayTime(void* data)
//{
//	PlayController* pc = (PlayController*)data;
//	if (!pc->pause())
//	{
//		//pc->playTimeSeconds()++;
//	}
//	Fl::repeat_timeout(0.001, PlayController::updatePlayTime, data);
//}

void PlayController::saveFrameToPng(AVFrame* frame, const char* path, AVCodecContext* codec_ctx) {
	//确保缓冲区长度大于图片,使用brga像素格式计算。如果是bmp或tiff依然可能超出长度，需要加一个头部长度，或直接乘以2。
	int bufSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, frame->width, frame->height, 64);
	//申请缓冲区
	uint8_t* buf = (uint8_t*)av_malloc(bufSize);
	//将视频帧转换成jpg图片，如果需要png则使用AV_CODEC_ID_PNG
	int picSize = frameToImage(frame, AV_CODEC_ID_PNG, buf, bufSize, nullptr);
	//写入文件
	FILE* f = nullptr;
	fopen_s(&f, path, "wb+");
	//auto f = fopen(path, "wb+");
	if (f)
	{
		fwrite(buf, sizeof(uint8_t), bufSize, f);
		fclose(f);
	}
	//释放缓冲区
	av_free(buf);
}


int PlayController::frameToImage(AVFrame* frame, enum AVCodecID codecID, uint8_t* outbuf, size_t outbufSize, AVCodecContext* codec_ctx)
{
	int ret = 0;
	AVPacket pkt;
	AVCodecContext* ctx = NULL;
	AVFrame* rgbFrame = NULL;
	uint8_t* buffer = NULL;
	struct SwsContext* swsContext = NULL;
	av_init_packet(&pkt);

    if(codec_ctx == nullptr)
    {
        const AVCodec* codec = avcodec_find_encoder(codecID);
        if (!codec)
        {
            printf("avcodec_send_frame error %d", codecID);
            goto end;
        }
        if (!codec->pix_fmts)
        {
            printf("unsupport pix format with codec %s", codec->name);
            goto end;
        }
        ctx = avcodec_alloc_context3(codec);
        ctx->bit_rate = 400000;
        ctx->width = frame->width;
        ctx->height = frame->height;
        ctx->time_base.num = 1;
        ctx->time_base.den = 25;
        ctx->gop_size = 10;
        ctx->max_b_frames = 0;
        ctx->thread_count = 1;
        ctx->pix_fmt = *codec->pix_fmts;
        ret = avcodec_open2(ctx, codec, NULL);
    }else{
        ctx = codec_ctx;
    }
	
	if (ret < 0)
	{
		printf("avcodec_open2 error %d", ret);
		goto end;
	}
	if (frame->format != ctx->pix_fmt)
	{
		rgbFrame = av_frame_alloc();
		if (rgbFrame == NULL)
		{
			printf("av_frame_alloc  fail");
			goto end;
		}
		swsContext = sws_getContext(frame->width, frame->height, (enum AVPixelFormat)frame->format, frame->width, frame->height, ctx->pix_fmt, 1, NULL, NULL, NULL);
		if (!swsContext)
		{
			printf("sws_getContext  fail");
			goto end;
		}
		int bufferSize = av_image_get_buffer_size(ctx->pix_fmt, frame->width, frame->height, 1) * 2;
		buffer = (unsigned char*)av_malloc(bufferSize);
		if (buffer == NULL)
		{
			printf("buffer alloc fail:%d", bufferSize);
			goto end;
		}
		av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, ctx->pix_fmt, frame->width, frame->height, 1);
		if ((ret = sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize)) < 0)
		{
			printf("sws_scale error %d", ret);
		}
		rgbFrame->format = ctx->pix_fmt;
		rgbFrame->width = ctx->width;
		rgbFrame->height = ctx->height;
		ret = avcodec_send_frame(ctx, rgbFrame);
	}
	else
	{
		ret = avcodec_send_frame(ctx, frame);
	}
	if (ret < 0)
	{
		printf("avcodec_send_frame error %d", ret);
		goto end;
	}
	ret = avcodec_receive_packet(ctx, &pkt);
	if (ret < 0)
	{
		printf("avcodec_receive_packet error %d", ret);
		goto end;
	}
	if (pkt.size > 0 && pkt.size <= outbufSize) 
		memcpy(outbuf, pkt.data, pkt.size);
	ret = pkt.size;
end:
	if (swsContext)
	{
		sws_freeContext(swsContext);
	}
	if (rgbFrame)
	{
		av_frame_unref(rgbFrame);
		av_frame_free(&rgbFrame);
	}
	if (buffer)
	{
		av_free(buffer);
	}
	av_packet_unref(&pkt);
	if (ctx)
	{
		avcodec_free_context(&ctx);
	}
	return ret;
}

void PlayController::threadWait()
{
	std::unique_lock<std::mutex> lock(m_syncMutex);
	m_isWaited = true;
	m_syncCond.wait(lock);
}

PlayController::PlayController()
{
	//m_playDecoder.reset(new EditDecoder());
	m_playDecoder.reset(new VideoDecoder());
}

PlayController::~PlayController()
{
	stopPlay();
	if (m_playDecoder)
	{
		m_playDecoder->stopDecode();
		//delete m_playDecoder;
	}
}

PlayStatus PlayController::status()
{
	if (m_pause == true)
	{
		return PlayStatus::pause;
	}
	if (m_stop == true)
	{
		return PlayStatus::stop;
	}
	return PlayStatus::playing;
}

double PlayController::playTimeSeconds()
{
	return (m_playStartTime) - m_pauseMS + m_seekTimeMs;
}

int PlayController::startPlay()
{
	m_playDecoder->setPlayController(this->shared_from_this());
	m_playDecoder->initDecode(m_url);
	if(m_playDecoder->m_formatContext) m_totalDuration = m_playDecoder->m_formatContext->duration / (double)AV_TIME_BASE;
	m_playDecoder->startDecode();

	m_playThread = new std::thread([this]() {
		//初始化播放时变量
		m_pause = false;
		m_stop = false;
		m_playTime = 0;
		m_playStartTime = 0;
		m_seekTimeMs = 0;
		//m_playDecoder->videoSeek(m_seekTimeMs);
		//setSeekTime(0);
		m_pauseMS = 0;

		m_playStart = std::chrono::high_resolution_clock::now();
		int initCount = 0;
		while (!m_stop)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1)); //1us处理1次

			//判断是否进入seek，阻塞解码线程，并让播放线程清理缓冲队列
			if (m_seekFlag)
			{
				//printf("播放线程进入seek逻辑\n");
				m_playDecoder->videoSeek(m_seekTimeMs);
				//printf("播放线程设置解码线程seek\n");
				//printf("播放线程尝试等待解码线程上锁\n");
				while (m_isWaited == false)
				{
					std::this_thread::sleep_for(std::chrono::microseconds(1));
				}
				//printf("播放线程检测到解码线程上锁\n");
				seek(m_seekTimeMs);
				//printf("播放线程seek初始化\n");
				m_playDecoder->freeBuffer();
				//printf("播放线程释放缓存\n");
				m_syncCond.notify_all();
				m_isWaited = false;
				//printf("播放线程唤醒解码线程\n");
				m_seekFlag = false;
				continue;
			}

			//如果用户停止播放，记录暂停的时间
			if (m_pause)
			{
				auto pause = std::chrono::high_resolution_clock::now();
				while (m_pause)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					if (m_stop) //如果设置停止，则退出从解码器获取帧
					{
						return;
					}
				}
				m_pauseMS += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - pause).count();
			}
			//*********************************************************

			initCount = 0;
			//等待解码器将解码出的frame放入队列
			while (m_playDecoder->videoFrameVector().empty() && m_seekFlag == false)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				//printf("等待解码初始化中...%dms\n", (++initCount) * 10);
				if (m_stop) //如果设置停止，则退出从解码器获取帧
				{
					return;
				}
			}
			//************************************************************

			//视频帧缓存队列
			if (m_playDecoder->videoFrameVector().empty() == false)
			{
				AVFrame* vFrame = m_playDecoder->videoFrameVector().front();
				//判断时间戳，是否缓存帧已经可以播放
				if (m_playTime >= vFrame->pts * m_playDecoder->videoTimeBaseMs())
				{
					double frameMs = vFrame->pts * m_playDecoder->videoTimeBaseMs();
					if (m_func && !m_stop) 
						m_func(vFrame, m_v, (int)frameMs);

					if (vFrame && !m_stop)
					{
						//std::cout << "play one frame tine = " << m_playTime << std::endl;	
						av_frame_free(&vFrame);
						if (m_playDecoder->videoFrameVector().size() > 0)
							m_playDecoder->videoFrameVector().pop_front();
					}
				}
			}

			//更新播放时间戳
			m_playStartTime = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_playStart).count() * m_playSpeed);
			m_playTime = m_playStartTime - m_pauseMS + m_seekTimeMs;
		}

		m_pause = false;
		m_stop = true;
	});


    return 0;
}

void PlayController::stopPlay()
{
	if (m_playThread && m_playThread->joinable())
	{
		m_stop = true;
		printf("wait thread exit...\n");
		m_playThread->join();
		printf("play controller thread exit\n");
		if (m_playDecoder)
		{
			m_playDecoder->stopDecode();
		}
		
		delete m_playThread;
		m_playThread = nullptr;


	}
}
