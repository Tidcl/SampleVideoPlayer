#include "EditDecoder.h"

EditDecoder::EditDecoder()
{

}

EditDecoder::~EditDecoder()
{

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

	int frame_count = 0;
	m_stopDecode = false;

	double lastFrameTimeStep = 0;
	int read_frame_rtn = av_read_frame(m_formatContext, packet);
	int duration_pts = -1;

	int loopCount = 0;	//�ڼ���ѭ��
	int loopMaxPts = 0;	//���pts

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
						//��ͣ����
						//double backTime = bufferFrame->pts * videoTimeBaseMs();//��ǰ֡��ʱ�����
						//if (backTime > m_playController->playTimeSeconds())
						//{
							//����һ֡����
							AVFrame* tempFrame = av_frame_alloc();
							if (tempFrame)
							{
								if (av_frame_ref(tempFrame, bufferFrame) < 0)
								{
									av_frame_free(&tempFrame);
								}

								tempFrame->pts = tempFrame->pts + loopMaxPts * loopCount;	//����ѭ�����������µ�pts

								//�жϻ����е�֡���ݣ��Ƿ����㲥��ʱ���+����ʱ��
								if (m_videoFrameVec.empty())
								{
									m_videoFrameVec.push_back(tempFrame);
								}
								else
								{
									while (m_videoFrameVec.size() > m_bufFrameCount && m_seekFlag == false)
									{
										if (m_stopDecode) break;
										std::this_thread::sleep_for(std::chrono::microseconds(1));
									}

									if (!m_seekFlag) m_videoFrameVec.push_back(tempFrame);
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

