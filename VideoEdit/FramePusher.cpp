#include "FramePusher.h"
#include <windows.h>

FramePusher::FramePusher()
{
	avformat_network_init();
}

FramePusher::~FramePusher()
{
	stopPush();
}

void FramePusher::updateMat(cv::Mat mat)
{
	m_updateMat = mat;
	m_updateFlag = true;
}

int FramePusher::initFFmpeg()
{
	//::CoUninitialize();
	// 输出格式设置为RTMP推流
	AVFormatContext* fmt_ctx = nullptr;
	avformat_alloc_output_context2(&fmt_ctx, nullptr, "flv", m_serverPath.c_str());
	if (!fmt_ctx) {
		std::cerr << "无法分配输出格式上下文" << std::endl;
		return -1;
	}

	// 设置编码器
	const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	//const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
	if (!codec) {
		std::cerr << "找不到H264编码器" << std::endl;
		return -1;
	}

	AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx) {
		std::cerr << "无法分配编码器上下文" << std::endl;
		return -1;
	}

	codec_ctx->width = m_width;
	codec_ctx->height = m_height;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->time_base = AVRational{ 1, m_frameRate }; // 每秒帧
	codec_ctx->framerate = AVRational{ m_frameRate, 1 };
	codec_ctx->gop_size = 10;
	codec_ctx->max_b_frames = 1;
	codec_ctx->bit_rate = m_bitRate;

	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
		codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	int rtn = 0;
 	if ((rtn = avcodec_open2(codec_ctx, codec, nullptr)) < 0) {
		std::cerr << "无法打开编码器" << std::endl;
		return -1;
	}

	m_fmt_ctx.reset(fmt_ctx, [](AVFormatContext* ptr) {
		avformat_free_context(ptr);
		});
	m_codec_ctx.reset(codec_ctx, [](AVCodecContext* ptr) {
		avcodec_free_context(&ptr);
		});
	return 0;
}

int FramePusher::pushing()
{
	AVFormatContext* fmt_ctx = m_fmt_ctx.get();
	AVCodecContext* codec_ctx = m_codec_ctx.get();

	// 设置视频流
	AVStream* stream = avformat_new_stream(fmt_ctx, nullptr);
	if (!stream) {
		std::cerr << "无法创建视频流" << std::endl;
		return -1;
	}

	avcodec_parameters_from_context(stream->codecpar, codec_ctx);

	// 打开输出URL
	if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		if (avio_open(&fmt_ctx->pb, fmt_ctx->url, AVIO_FLAG_WRITE) < 0) {
			std::cerr << "无法打开输出URL" << std::endl;
			return -1;
		}
	}

	// 写文件头
	if (avformat_write_header(fmt_ctx, nullptr) < 0) {
		std::cerr << "无法写入文件头" << std::endl;
		return -1;
	}

	// 初始化图像转换上下文
	SwsContext* sws_ctx = sws_getContext(
		codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24,
		codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	std::shared_ptr<SwsContext> sws_ctx_t(sws_ctx, [](SwsContext* ptr) {sws_freeContext(ptr); });

	AVFrame* frame = av_frame_alloc();
	std::shared_ptr<AVFrame> frame_t(frame, [](AVFrame* ptr) {av_frame_free(&ptr); });
	frame->format = codec_ctx->pix_fmt;
	frame->width = codec_ctx->width;
	frame->height = codec_ctx->height;
	av_frame_get_buffer(frame, 0);

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;

	int frame_index = 0;

	//cv::Mat pushMat;
	cv::Mat img;
	img = m_updateMat;

	while (!m_stopPullFlag) {
		if (m_updateFlag)
		{
			img = m_updateMat;
			m_updateFlag = false;
		}
		if (img.empty()) continue;

		// 将BGR24转换为YUV420P
		uint8_t* inData[1] = { img.data };
		int inLinesize[1] = { static_cast<int>(img.step) };
		sws_scale(sws_ctx, inData, inLinesize, 0, codec_ctx->height, frame->data, frame->linesize);

		frame->pts = frame_index++;

		if (avcodec_send_frame(codec_ctx, frame) < 0) {
			std::cerr << "无法发送帧到编码器" << std::endl;
			return -1;
		}

		if (avcodec_receive_packet(codec_ctx, &pkt) == 0) {
			pkt.stream_index = stream->index;
			av_packet_rescale_ts(&pkt, codec_ctx->time_base, stream->time_base);
			pkt.pos = -1;

			if (av_interleaved_write_frame(fmt_ctx, &pkt) < 0) {
				std::cerr << "无法写入帧" << std::endl;
				//return -1;
			}

			av_packet_unref(&pkt);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	// 写文件尾
	av_write_trailer(fmt_ctx);
}



void FramePusher::startPush()
{
	std::thread* t = new std::thread(std::bind(&FramePusher::pushing, this));
	m_pullThread.reset(t);
}

void FramePusher::stopPush()
{
	m_stopPullFlag = true;
	if (m_pullThread.get() && m_pullThread->joinable())
	{
		m_pullThread->join();
	}
}
