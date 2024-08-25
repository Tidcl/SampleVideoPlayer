#include <windows.h>
#include <chrono>
#include <thread>
#include "PlayDecoder.h"
#include "PlayController.h"

void PlayDecoder::initDecode(std::string url)
{
	freeDecode();

	const char* video_file = url.c_str();

	//avformat_network_init();

	// av_register_all();
	if (strlen(video_file) == 0) return;

	if (avformat_open_input(&m_formatContext, video_file, nullptr, nullptr) != 0) {
		std::cout << "Cannot open video file: " << video_file << std::endl;
		return;
	}

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

	if (audio_stream_index != -1)
	{
		audio_time_base = m_formatContext->streams[audio_stream_index]->time_base;
		audio_codecpar = m_formatContext->streams[audio_stream_index]->codecpar;
		audio_codec = const_cast<AVCodec*>(avcodec_find_decoder(audio_codecpar->codec_id));
		if (!audio_codec) {
			std::cout << "Unsupported codec" << std::endl;
			return;
		}
		audio_context = avcodec_alloc_context3(audio_codec);
		if (!audio_context) {
			std::cout << "Cannot allocate codec context" << std::endl;
			return;
		}
		if (avcodec_parameters_to_context(audio_context, audio_codecpar) < 0) {
			std::cout << "Cannot copy codec parameters to context" << std::endl;
			return;
		}
		if (avcodec_open2(audio_context, audio_codec, nullptr) < 0) {
			std::cout << "Cannot open codec" << std::endl;
			return;
		}
	}
	else if (audio_stream_index == -1) {
		std::cout << "No video stream found" << std::endl;
		return;
	}


	for (unsigned int i = 0; i < m_formatContext->nb_streams; i++) {
		if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			break;
		}
	}

	if (video_stream_index != -1)
	{
		video_time_base = m_formatContext->streams[video_stream_index]->time_base;
		video_codecpar = m_formatContext->streams[video_stream_index]->codecpar;
		video_codec = const_cast<AVCodec*>(avcodec_find_decoder(video_codecpar->codec_id));
		if (!video_codec) {
			std::cout << "Unsupported codec" << std::endl;
			return;
		}

		video_context = avcodec_alloc_context3(video_codec);
		if (!video_context) {
			std::cout << "Cannot allocate codec context" << std::endl;
			return;
		}

		if (avcodec_parameters_to_context(video_context, video_codecpar) < 0) {
			std::cout << "Cannot copy codec parameters to context" << std::endl;
			return;
		}

		if (avcodec_open2(video_context, video_codec, nullptr) < 0) {
			std::cout << "Cannot open codec" << std::endl;
			return;
		}
	} else if (video_stream_index == -1) {
		std::cout << "No video stream found" << std::endl;
		return;
	}
}

void PlayDecoder::freeDecode()
{
	audio_codecpar = nullptr;
	audio_codec = nullptr;
	audio_stream_index = -1;

	video_codecpar = nullptr;
	video_codec = nullptr;
	video_stream_index = -1;

	//avcodec_free_context(&audio_context);
	//avcodec_free_context(&video_context);
	avformat_close_input(&m_formatContext);

	m_formatContext = nullptr;
	video_context = nullptr;
	audio_context = nullptr;
}

void PlayDecoder::startDecode()
{
	m_stopDecode = false;
	m_decodeThread = new std::thread([this]() {
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

		while (av_read_frame(m_formatContext, packet) >= 0) {
			if (m_stopDecode) break;

			if (packet->stream_index == video_stream_index) {
				if (video_context && avcodec_send_packet(video_context, packet) == 0) {
					while (avcodec_receive_frame(video_context, bufferFrame) == 0) {
						if (m_stopDecode) break;
						//暂停解码
						double backTime = bufferFrame->pts * videoTimeBase();//当前帧的时间戳秒

						if (backTime > m_playController->playTimeSeconds())
						{
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
									std::unique_lock<std::mutex> guard(m_mutex);
									m_videoFrameVec.push_back(tempFrame);
									//printf("empty push frame time step = %lf\n", backTime);
								}
								else
								{
									//缓冲中最后一帧的时间戳
									double lastFrameTimeStep = (*(m_videoFrameVec.rbegin()))->pts * videoTimeBase();
									while (lastFrameTimeStep > (m_playController->playTimeSeconds() + m_bufferTime))
									{
										if (m_stopDecode) break;
										if (m_videoFrameVec.size() == 0)
										{
											lastFrameTimeStep = -1;	//退出等待播放时间戳前进的循环。如果在解码前发现缓存中存在数据，且最后一帧时间戳远大于播放时间+缓存时间，便会进入循环等待播放时间戳前进。如果播放时间偏移是向前偏移，则会出现永远也不会出现当前帧在缓存时间中。
										}
										std::this_thread::sleep_for(std::chrono::microseconds(1));
									}
									std::unique_lock<std::mutex> guard(m_mutex);
									//printf("push frame time step = %lf\n", backTime);
									if (m_videoFrameVec.size() > 0) m_videoFrameVec.push_back(tempFrame);
								}
							}
						}
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
	});

	
}

void PlayDecoder::stopDecode()
{
	if (m_decodeThread && m_decodeThread->joinable())
	{
		m_stopDecode = true;
		printf("等待线程退出...\n");
		m_decodeThread->join();
		printf("解码线程退出成功\n");
		delete m_decodeThread;
		m_decodeThread = nullptr;
		m_videoFrameVec.clear();
		m_audioFrameVec.clear();
	}
}

void PlayDecoder::videoSeek(long timeMs)
{
	if (m_formatContext == nullptr) return;

	std::unique_lock<std::mutex> guard(m_mutex);
	long timeStep = timeMs / videoTimeBase();

	if (av_seek_frame(m_formatContext, video_stream_index, timeStep, AVSEEK_FLAG_BACKWARD) < 0) {
		fprintf(stderr, "Could not seek to position.\n");
	}
	else 
	{
		m_videoFrameVec.clear();
		m_audioFrameVec.clear();
		//avcodec_parameters_to_context(video_context, m_formatContext->streams[video_stream_index]->codecpar);
		//avcodec_flush_buffers(video_context);
	}
}

bool PlayDecoder::isFree()
{
	return m_formatContext == nullptr?true : false;
}

