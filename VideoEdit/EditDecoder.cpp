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

	int loopCount = 0;	//第几次循环
	int loopMaxPts = 0;	//最大pts

	if (read_frame_rtn >= 0)	//如果第一次读取都失败，直接退出，否则进入循环处理
	{
		while (read_frame_rtn >= 0) 
		{
			if (m_seekFlag)	//如果要求解码器seek寻找
			{
				m_playController->threadWait();		//等待直到播放线程也进入ready状态，当播放线程进入ready状态进行notify。解码线程再去重新解码
				//当播放线程进入ready会清除缓存队列
				m_seekFlag = false;
			}

			if (m_stopDecode) break;	//要求解码线程退出

			if (packet->stream_index == video_stream_index) {
				if (video_context && avcodec_send_packet(video_context, packet) == 0) {
					while (avcodec_receive_frame(video_context, bufferFrame) == 0) {	//从解码器中取出解码后的frame
						if (m_stopDecode) break;
						loopMaxPts = bufferFrame->pts > loopMaxPts ? bufferFrame->pts : loopMaxPts;
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

								tempFrame->pts = tempFrame->pts + loopMaxPts * loopCount;	//根据循环次数计算新的pts

								//判断缓存中的帧数据，是否满足播放时间戳+缓冲时间
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
			//已经进入循环处理，当read_frame错误，就需要重置seek
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

