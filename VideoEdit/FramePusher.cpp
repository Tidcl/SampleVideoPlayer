#include "FramePusher.h"
#include <windows.h>
#include "VideoPlay/PlayController.h"

FramePusher::FramePusher()
{
	avformat_network_init();
}

FramePusher::~FramePusher()
{
	stopPush();
}

void FramePusher::setPushFPS(int fps)
{
	m_frameRate = fps;
}

int FramePusher::FPS()
{
	return m_frameRate;
}

void FramePusher::updateFrame(cv::Mat& mat)
{
	std::swap(m_updateMat, mat);
	m_updateFlag = true;
}

void FramePusher::pushFrame(cv::Mat& mat)
{
	std::lock_guard<std::mutex> guard(m_mutex);
	m_pushVector.push_back(mat);
}

void FramePusher::clearBuffer()
{
	m_pushVector.clear();
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

	// 写文件头
	if (avformat_write_header(fmt_ctx, nullptr) < 0) {
		std::cerr << "无法写入文件头" << std::endl;
		return -1;
	}

	// 初始化图像转换上下文
	AVFrame* frame = av_frame_alloc();
	std::shared_ptr<AVFrame> frame_t(frame, [](AVFrame* ptr) {av_frame_free(&ptr); });
	frame->format = codec_ctx->pix_fmt;
	frame->width = codec_ctx->width;
	frame->height = codec_ctx->height;
	av_frame_get_buffer(frame, 32);

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;

	int frame_index = 0;

	cv::Mat img;
	m_stopPullFlag = false;

	int frameCount = 0;
	double frameTime = 1e3 / m_frameRate;

	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
	while (!m_stopPullFlag) {

		//每次循环发送一帧给编码器
		if(m_pushVector.empty() == false)
		{
			std::lock_guard<std::mutex> guard(m_mutex);
			cv::Mat mat = m_pushVector.front();
			m_pushVector.erase(m_pushVector.begin());
			std::swap(img, mat);
		}

		if (img.empty() == false)
		{
			uint8_t* inData[1] = { img.data };
			int inLinesize[1] = { static_cast<int>(img.step) };
			resizeSws(img);	// 重置sws的转换分辨率,将img转为配置的成员宽高
			sws_scale(m_sws_ctx.get(), inData, inLinesize, 0, img.rows, frame->data, frame->linesize);
			frame->pts = frame_index++;	// 设置帧的 PTS
			if (avcodec_send_frame(codec_ctx, frame) < 0) {
				std::cerr << "无法发送帧到编码器" << std::endl;
			}


			//从编码器取出编码好的一帧
			int rtn = 0;
			while ((rtn = avcodec_receive_packet(codec_ctx, &pkt)) == 0) {
				pkt.stream_index = stream->index;
				pkt.pts = av_rescale_q(pkt.pts, codec_ctx->time_base, stream->time_base);
				pkt.dts = av_rescale_q(pkt.dts, codec_ctx->time_base, stream->time_base);
				pkt.duration = av_rescale_q(pkt.duration, codec_ctx->time_base, stream->time_base);
				pkt.pos = -1;

				if (av_interleaved_write_frame(fmt_ctx, &pkt) < 0) {
					std::cerr << "无法写入帧" << std::endl;
					//break;
					goto end;
				}
				av_packet_unref(&pkt);
			}
			img.release();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		long long frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		int64_t next_frame_time = frameCount++ * frameTime;
		if (next_frame_time > frame_time) {
			std::this_thread::sleep_for(std::chrono::milliseconds(next_frame_time - frame_time));
		}
	}
end:
	av_write_trailer(fmt_ctx);	// 写文件尾
}



int FramePusher::openCodec(int width, int height, int fps)
{
	//stopPush();
	m_width = width;
	m_height = height;
	m_frameRate = fps;

	// 输出格式设置为RTMP推流
	m_initSuccessful = false;
	AVFormatContext* fmt_ctx = nullptr;
	avformat_alloc_output_context2(&fmt_ctx, nullptr, "flv", m_serverPath.c_str());
	if (!fmt_ctx) {
		std::cerr << "无法分配输出格式上下文" << std::endl;
		return -1;
	}

	// 设置编码器
	const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
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
	codec_ctx->codec_id = AV_CODEC_ID_H264;
	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	int ration = 1;
	codec_ctx->time_base = AVRational{ 1 * ration, m_frameRate * ration }; // 每秒帧
	codec_ctx->framerate = AVRational{ m_frameRate * ration, 1 * ration };
	codec_ctx->gop_size = 1;
	codec_ctx->max_b_frames = 0;
	codec_ctx->bit_rate = m_bitRate;

	if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
		codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	int rtn = 0;
	if ((rtn = avcodec_open2(codec_ctx, codec, 0)) < 0) {
		std::cerr << "无法打开编码器" << std::endl;
		return -1;
	}

	// 打开输出URL
	if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		if (avio_open(&fmt_ctx->pb, fmt_ctx->url, AVIO_FLAG_WRITE) < 0) {
			std::cerr << "无法打开输出URL" << std::endl;
			avformat_free_context(fmt_ctx);
			avcodec_free_context(&codec_ctx);
			return -1;
		}
	}


	m_fmt_ctx.reset(fmt_ctx, [](AVFormatContext* ptr) { avformat_free_context(ptr); });
	m_codec_ctx.reset(codec_ctx, [](AVCodecContext* ptr) { avcodec_free_context(&ptr); });

	av_dump_format(fmt_ctx, 0, m_serverPath.c_str(), 1);
	m_initSuccessful = true;
	return 0;
}

void FramePusher::closeCodec()
{
	if (m_codec_ctx)
		m_codec_ctx.reset();

	if(m_fmt_ctx)
		m_fmt_ctx.reset();
}	

void FramePusher::resizeSws(cv::Mat& img)
{
	static int sws_width = 0;
	static int sws_height = 0;

	if (img.cols != sws_width || img.rows != sws_height)
	{
		sws_width = img.cols;
		sws_height = img.rows;
		SwsContext* new_sws_ctx = sws_getContext(
			sws_width, sws_height, AV_PIX_FMT_BGR24,
			m_codec_ctx->width, m_codec_ctx->height, m_codec_ctx->pix_fmt,
			SWS_BILINEAR, nullptr, nullptr, nullptr);
		m_sws_ctx.reset(new_sws_ctx, [](SwsContext* ptr) {sws_freeContext(ptr); });
	}
}

void FramePusher::startPush()
{
	std::thread* t = new std::thread(std::bind(&FramePusher::pushing, this));
	m_pullThread.reset(t);
}

void FramePusher::stopPush()
{
	m_stopPullFlag = true;
	if (m_pullThread && m_pullThread->joinable())
	{
		m_pullThread->join();
		m_initSuccessful = false;
		closeCodec();
		clearBuffer();
	}
}

int FramePusher::bufferCount()
{
	return m_pushVector.size();
}

bool FramePusher::isInitSuccessful()
{
	return m_initSuccessful;
}

