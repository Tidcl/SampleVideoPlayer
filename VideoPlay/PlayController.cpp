#include "PlayController.h"
#include "VideoDecoder.h"
#include "VideoEdit/EditDecoder.h"

void PlayController::seek(long timeMS)
{
	if(m_playDecoder) m_playDecoder->videoSeek(timeMS);
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
	m_timer.setInterval(m_timerIntervalMS / m_playSpeed);
}

void PlayController::setFrameRecall(frameRecall* func, void* v)
{
	m_func = func;
	m_v = v;
};

void saveFrameToPng(AVFrame* frame, const char* path, AVCodecContext* codec_ctx) {
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


int frameToImage(AVFrame* frame, enum AVCodecID codecID, uint8_t* outbuf, size_t outbufSize, AVCodecContext* codec_ctx)
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

AVFrame& PlayController::lastFrame()
{
	return m_lastFrame;
}

void PlayController::threadWait()
{
	m_isCodecoWaited = true;
	while (m_isCodecoWaited)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

PlayController::PlayController()
{
	m_playDecoder.reset(new EditDecoder());
	//m_playDecoder.reset(new VideoDecoder());
}

PlayController::~PlayController()
{
	stopPlayByTimer();
}

void PlayController::setVideoUrl(std::string url)
{
	m_url = url;
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

int PlayController::startPlayByTimer()
{
	m_playDecoder->setPlayController(this->shared_from_this());
	m_playDecoder->initDecode(m_url);
	m_timerIntervalMS = m_playDecoder->videoFps();
	if (m_playDecoder->m_formatContext) m_totalDuration = m_playDecoder->m_formatContext->duration / (double)AV_TIME_BASE;
	m_playDecoder->startDecode();

	m_pause = false;
	m_stop = false;
	m_timer.setInterval(m_timerIntervalMS);
	m_timer.setCallbackFun(&PlayController::timerCallBack, this);
	m_timer.start();

	return 0;
}

void PlayController::stopPlayByTimer()
{
	m_timer.stop();

	if (m_playDecoder)
	{
		m_playDecoder->stopDecode();
	}
}

void PlayController::timerCallBack(void* val)
{
	PlayController* playC = (PlayController*)val;

	if (playC->m_seekFlag)
	{
		playC->m_playDecoder->videoSeek(playC->m_seekTimeMs);
		while (playC->m_isCodecoWaited == false && playC->m_playDecoder->m_stopDecode == false)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
		playC->m_playDecoder->freeBuffer();
		playC->m_isCodecoWaited = false;
		playC->m_seekFlag = false;
	}

	if (playC->m_playDecoder->videoFrameVector().empty())
	{
		return;
	}

	AVFrame* vFrame = playC->m_playDecoder->videoFrameVector().front();
	//判断时间戳，是否缓存帧已经可以播放
	double frameMs = vFrame->pts * playC->m_playDecoder->videoTimeBaseMs();
	if (playC->m_func && !playC->m_stop)
		playC->m_func(vFrame, playC->m_v, (int)frameMs);

	playC->m_lastFrame = *vFrame;
	if (vFrame && !playC->m_stop)
	{
		av_frame_free(&vFrame);
		if (playC->m_playDecoder->videoFrameVector().size() > 0)
			playC->m_playDecoder->videoFrameVector().pop_front();
	}
}
