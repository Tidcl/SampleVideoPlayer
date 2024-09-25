#include "VideoDecoder.h"
#include "PlayController.h"

VideoDecoder::VideoDecoder()
{

}

VideoDecoder::~VideoDecoder()
{

}

void VideoDecoder::setPlayController(std::shared_ptr<PlayController> controller)
{
	m_playController = controller;
}

void VideoDecoder::initDecode(std::string url)
{
	freeDecode();
	freeBuffer();

	const char* video_file = url.c_str();

	// avformat_network_init();

	// av_register_all();
	if (strlen(video_file) == 0) return;

	//打开文件
	if (avformat_open_input(&m_formatContext, video_file, nullptr, nullptr) != 0) {
		std::cout << "Cannot open video file: " << video_file << std::endl;
		return;
	}

	//找到音频流
	if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
		std::cout << "Cannot find stream information" << std::endl;
		return;
	}
	for (unsigned int i = 0; i < m_formatContext->nb_streams; i++) {
		if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_index = i;
			break;
		}
	}

	//初始化音频解码器、复用器
	if (audio_stream_index != -1)
	{
		audio_time_base = m_formatContext->streams[audio_stream_index]->time_base;			//音频时间基
		audio_codecpar = m_formatContext->streams[audio_stream_index]->codecpar;			//解码器参数
		audio_codec = const_cast<AVCodec*>(avcodec_find_decoder(audio_codecpar->codec_id));	//解码器
		if (!audio_codec) {
			std::cout << "Unsupported codec" << std::endl;
			return;
		}
		audio_context = avcodec_alloc_context3(audio_codec);
		if (!audio_context) {
			std::cout << "Cannot allocate codec context" << std::endl;
			return;
		}
		if (avcodec_parameters_to_context(audio_context, audio_codecpar) < 0) {			//从解码器参数复制到复用器
			std::cout << "Cannot copy codec parameters to context" << std::endl;
			return;
		}
		if (avcodec_open2(audio_context, audio_codec, nullptr) < 0) {				//打开解码器
			std::cout << "Cannot open codec" << std::endl;
			return;
		}
	}
	else if (audio_stream_index == -1) {
		std::cout << "No audio stream found" << std::endl;
	}

	//找到视频流
	for (unsigned int i = 0; i < m_formatContext->nb_streams; i++) {
		if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			break;
		}
	}
	if (video_stream_index != -1)
	{
		video_time_base = m_formatContext->streams[video_stream_index]->time_base;	//视频时间基
		video_codecpar = m_formatContext->streams[video_stream_index]->codecpar;	//视频解码器参数
		video_codec = const_cast<AVCodec*>(avcodec_find_decoder(video_codecpar->codec_id));	//视频解码器
		if (!video_codec) {
			std::cout << "Unsupported codec" << std::endl;
			return;
		}

		video_context = avcodec_alloc_context3(video_codec);
		if (!video_context) {
			std::cout << "Cannot allocate codec context" << std::endl;
			return;
		}

		if (avcodec_parameters_to_context(video_context, video_codecpar) < 0) {		//将视频解码器参数复制到复用器
			std::cout << "Cannot copy codec parameters to context" << std::endl;
			return;
		}

		if (avcodec_open2(video_context, video_codec, nullptr) < 0) {				//打开视频解码器
			std::cout << "Cannot open codec" << std::endl;
			return;
		}
	} else if (video_stream_index == -1) {
		std::cout << "No video stream found" << std::endl;
	}
}

void VideoDecoder::freeDecode()
{
	audio_codecpar = nullptr;
	audio_codec = nullptr;
	audio_stream_index = -1;

	video_codecpar = nullptr;
	video_codec = nullptr;
	video_stream_index = -1;


	if(m_formatContext) avformat_close_input(&m_formatContext);
	if(audio_context) avcodec_free_context(&audio_context);
	if(video_context) avcodec_free_context(&video_context);

	m_formatContext = nullptr;
	video_context = nullptr;
	audio_context = nullptr;
}

void VideoDecoder::startDecode()
{
	m_stopDecode = false;
	m_decodeThread = new std::thread(std::bind(&VideoDecoder::decode, this));
}

void VideoDecoder::stopDecode()
{
	if (m_decodeThread && m_decodeThread->joinable())
	{
		m_stopDecode = true;
		std::cout << "wait thread exit..." << std::endl;
		m_decodeThread->join();
		printf("decode thread exit successful\n");
		delete m_decodeThread;
		m_decodeThread = nullptr;
		freeBuffer();
		freeDecode();
	}
}

void VideoDecoder::videoSeek(long timeMs)
{
	if (m_formatContext == nullptr) return;

	m_seekFlag = true;
	long timeStep = (long)(timeMs / videoTimeBaseMs());

	if (av_seek_frame(m_formatContext, video_stream_index, timeStep, AVSEEK_FLAG_BACKWARD) < 0) {
		fprintf(stderr, "Could not seek to position.\n");
	}
	else 
	{
		//freeBuffer();
	}
}

bool VideoDecoder::isFree()
{
	return m_formatContext == nullptr?true : false;
}

void VideoDecoder::freeBuffer()
{
	AVFrame* frame = nullptr;
	while (m_videoFrameVec.empty() == false)
	{
		frame = m_videoFrameVec.front();
		av_frame_free(&frame);
		m_videoFrameVec.pop_front();
	}
	while (m_audioFrameVec.empty() == false)
	{
		frame = m_audioFrameVec.front();
		av_frame_free(&frame);
		m_audioFrameVec.pop_front();
	}
}

void VideoDecoder::decode()
{
	if (m_formatContext == nullptr) return;

	AVFrame* bufferFrame = av_frame_alloc();
	if (!bufferFrame) {
		std::cout << "Cannot allocate frame" << std::endl;
		return;
	}

	AVFrame* audioBufferFrame = av_frame_alloc();
	if (!audioBufferFrame) {
		std::cout << "Cannot allocate frame" << std::endl;
		return;
	}

	AVPacket* packet = av_packet_alloc();
	if (!packet)
	{
		std::cout << "Cannot allocate packet" << std::endl;
		return;
	}
	av_init_packet(packet);

	int frame_count = 0;
	m_stopDecode = false;

	double lastFrameTimeStep = 0;
	//int read_frame_rtn = 0;
	while (av_read_frame(m_formatContext, packet) >= 0) {

		if (m_seekFlag)	//如果要求解码器seek寻找
		{
			//printf("解码线程进入seek等待状态\n");
			m_playController->threadWait();		//等待直到播放线程也进入ready状态，当播放线程进入ready状态进行notify。解码线程再去重新解码
			//当播放线程进入ready会清除缓存队列
			//printf("解码线程seek等待结束\n");
			m_seekFlag = false;
		}


		if (m_stopDecode) break;	//要求解码线程退出

		if (packet->stream_index == video_stream_index) {
			if (video_context && avcodec_send_packet(video_context, packet) == 0) {
				while (avcodec_receive_frame(video_context, bufferFrame) == 0) {	//从解码器中取出解码后的frame
					if (m_stopDecode) break;
					//暂停解码
					//double backTime = bufferFrame->pts * videoTimeBaseMs();//当前帧的时间戳秒
					//if (backTime > m_playController->playTimeSeconds())
					//{
						//复制一帧数据
						AVFrame* tempFrame = av_frame_alloc();
						if (tempFrame)
						{
							if (av_frame_ref(tempFrame, bufferFrame) < 0)
							{
								av_frame_free(&tempFrame);
							}
								
							//判断缓存中的帧数据，是否满足播放时间戳+缓冲时间
							if (m_videoFrameVec.empty())
							{
								m_videoFrameVec.push_back(tempFrame);
								//lastFrameTimeStep = tempFrame->pts * videoTimeBaseMs();
								//printf("empty push frame time step = %lf\n", backTime);
							}
							else
							{
								while (m_videoFrameVec.size() > m_bufFrameCount && m_seekFlag == false)
								{
									if (m_stopDecode) break;
									std::this_thread::sleep_for(std::chrono::microseconds(1));
								}

								if(!m_seekFlag) m_videoFrameVec.push_back(tempFrame);

								//缓冲队列中最后一帧的时间戳
								//double lastFrameTimeStep = (*(m_videoFrameVec.rbegin()))->pts * videoTimeBaseMs();
								//while (lastFrameTimeStep > (m_playController->playTimeSeconds() + m_bufferTime))
								//{
								//	if (m_stopDecode) break;
								//	if (m_videoFrameVec.size() == 0 || m_seekFlag)
								//	{
								//		lastFrameTimeStep = -1;	//退出等待播放时间戳前进的循环。如果在解码前发现缓存中存在数据，且最后一帧时间戳远大于播放时间+缓存时间，便会进入循环等待播放时间戳前进。如果播放时间偏移是向前偏移，则会出现永远也不会出现当前帧在缓存时间中。
								//	}
								//	std::this_thread::sleep_for(std::chrono::microseconds(1));
								//}
								////std::unique_lock<std::mutex> guard(m_mutex);
								////printf("push frame time step = %lf\n", backTime);
								//if (lastFrameTimeStep != -1 && m_videoFrameVec.size() > 0) { 
								//	m_videoFrameVec.push_back(tempFrame); 
								//	lastFrameTimeStep = tempFrame->pts * videoTimeBaseMs();
								//}
								//else av_frame_free(&tempFrame);
							}
						}
					//}
				}
			}
		}
		else if (packet->stream_index == audio_stream_index)
		{
			if (audio_context && avcodec_send_packet(audio_context, packet) == 0) {
				while (avcodec_receive_frame(audio_context, audioBufferFrame) == 0) {
					if (m_stopDecode) break;
					//AVFrame* tempFrame = av_frame_alloc();
					//if (av_frame_ref(tempFrame, audioBufferFrame) < 0)
					//{
					//	av_frame_free(&tempFrame);
					//}
					//m_audioFrameVec.push_back(tempFrame);
				}
			}
		}
		av_packet_unref(packet);
	}

	av_packet_free(&packet);
	av_frame_free(&bufferFrame);
	av_frame_free(&audioBufferFrame);
	freeDecode();
}

