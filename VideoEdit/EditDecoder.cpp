#include "EditDecoder.h"

EditDecoder::EditDecoder()
{

}

EditDecoder::~EditDecoder()
{
	//stopDecode();
}

void EditDecoder::setStartPlayTime(int time)
{
	m_startPlayTime = time;
}


cv::Mat EditDecoder::popFrontMat()
{
	if (m_videoFrameVec.empty()) return cv::Mat();
	
	AVFrame* avframe = nullptr;
	{
		std::lock_guard<std::mutex> guard(m_mutex);
		avframe = m_videoFrameVec.front();
		m_videoFrameVec.pop_front();
	}
	//cv::Mat mat = AVFrameToMat(frontFrame);
	struct SwsContext* sws_ctx = sws_getContext(
		avframe->width, avframe->height, static_cast<AVPixelFormat>(avframe->format),
		m_width, m_height, AV_PIX_FMT_BGR24,
		SWS_BILINEAR, NULL, NULL, NULL);

	cv::Mat mat(m_height, m_width, CV_8UC3);
	// ִ��ת��
	uint8_t* dest[4] = { mat.data, NULL, NULL, NULL };
	int dest_linesize[4] = { mat.step[0], 0, 0, 0 };
	sws_scale(sws_ctx, avframe->data, avframe->linesize, 0, avframe->height, dest, dest_linesize);
	
	//return mat;
	av_frame_free(&avframe);
	sws_freeContext(sws_ctx);
	
	return mat;
}

double EditDecoder::frontTimeStep()
{
	if (m_videoFrameVec.empty())
	{
		return -1;
	}
	else
	{
		return m_videoFrameVec.front()->pts * videoTimeBaseMs();
	}
}

void EditDecoder::decode()
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

	
	int duration_pts = -1;
	int loopCount = 0;	//�ڼ���ѭ��
	int loopMaxPts = 0;	//���pts

	int read_frame_rtn = av_read_frame(m_formatContext, packet);
	if (read_frame_rtn >= 0)	//�����һ�ζ�ȡ��ʧ�ܣ�ֱ���˳����������ѭ������
	{
		while (read_frame_rtn >= 0) 
		{
			if (m_seekFlag)	//���Ҫ�������seekѰ��
			{
				m_playController->threadWait();		//�ȴ�ֱ�������߳�Ҳ����ready״̬���������߳̽���ready״̬����notify�������߳���ȥ���½���
				//�������߳̽���ready������������
				m_seekFlag = false;
			}

			if (m_stopDecode) break;	//Ҫ������߳��˳�

			if (packet->stream_index == video_stream_index) {
				if (video_context && avcodec_send_packet(video_context, packet) == 0) {
					while (avcodec_receive_frame(video_context, bufferFrame) == 0) {	//�ӽ�������ȡ��������frame
						if (m_stopDecode) break;
						loopMaxPts = bufferFrame->pts > loopMaxPts ? bufferFrame->pts : loopMaxPts;

						//����һ֡����
						AVFrame* tempFrame = av_frame_alloc();
						if (tempFrame)
						{
							if (av_frame_ref(tempFrame, bufferFrame) < 0)
							{
								av_frame_free(&tempFrame);
							}

							tempFrame->pts = tempFrame->pts + loopMaxPts * loopCount;	//����ѭ�����������µ�pts
							//tempFrame->pts += (m_startPlayTime * (tempFrame->time_base.den / tempFrame->time_base.num)); //���ݿ�ʼ����ʱ���time_base���˳�startPts

							//�жϻ����е�֡���ݣ��Ƿ����㲥��ʱ���+����ʱ��
							if (m_videoFrameVec.empty())
							{
								std::lock_guard<std::mutex> guard(m_mutex);
								m_videoFrameVec.push_back(tempFrame);
							}
							else
							{
								while (m_videoFrameVec.size() >= m_bufFrameCount && m_seekFlag == false)
								{
									if (m_stopDecode) break;
									std::this_thread::sleep_for(std::chrono::milliseconds(1));
								}

								if (!m_seekFlag)
								{
									std::lock_guard<std::mutex> guard(m_mutex);
									m_videoFrameVec.push_back(tempFrame);
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
					}
				}
			}
			av_packet_unref(packet);

			read_frame_rtn = av_read_frame(m_formatContext, packet);
			//�Ѿ�����ѭ��������read_frame���󣬾���Ҫ����seek
			if (read_frame_rtn != 0)
			{
				long timeStep = (long)(0 / videoTimeBaseMs());
				if (av_seek_frame(m_formatContext, video_stream_index, timeStep, AVSEEK_FLAG_BACKWARD) < 0) {
					fprintf(stderr, "Could not seek to position.\n");
				}
				else
				{
					read_frame_rtn = av_read_frame(m_formatContext, packet);
					loopCount += 1;
				}
			}
		}
	}
	
	av_packet_free(&packet);
	av_frame_free(&bufferFrame);
	av_frame_free(&audioBufferFrame);
	freeDecode();
}

