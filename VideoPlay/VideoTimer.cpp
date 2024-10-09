#include "VideoTimer.h"

VideoTimer::VideoTimer()
{
}

VideoTimer::~VideoTimer()
{
	stop();
}

void VideoTimer::setInterval(double intervalms)
{
	m_intervalms = intervalms;
}

void VideoTimer::setCallbackFun(VideoTimerFun func, void* value)
{
	m_ffun = std::bind(func, value);
	m_func = func;
}

void VideoTimer::start()
{
	if (m_intervalms == -1) return;

	std::thread* timerThread = new std::thread([&]() {
		std::chrono::steady_clock::time_point start;

		long long duration = 0;
		long long leftTimeUs = 0;
		long long tempLoopCount = 1;
		long long waiteTime = 0;
		start = std::chrono::high_resolution_clock::now();

		while (m_stop == false)
		{
			while (m_pause)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			
			if (m_func) m_ffun();

			duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
			leftTimeUs = m_intervalms * m_loopCount - duration;	//����ʣ���΢��
			m_loopCount++;
			if (leftTimeUs > 0)	//����Ҫ�ȴ�
			{
				std::this_thread::sleep_for(std::chrono::milliseconds((long)leftTimeUs));
			}
			else if (leftTimeUs < 0) //�������ʱ��
			{
				leftTimeUs = -leftTimeUs;
				auto left = leftTimeUs % (long)m_intervalms;
				if (left == 0)
				{
					m_loopCount += leftTimeUs / m_intervalms;
				}
				else
				{
					m_loopCount += leftTimeUs / m_intervalms;
					waiteTime = m_intervalms - left;
					std::this_thread::sleep_for(std::chrono::milliseconds(waiteTime));
					//std::cout << "us   m_interval " << m_intervalms * 1000 << "    duration " << duration << "     waiteTime " << waiteTime << "    tempLoopCount " << tempLoopCount << std::endl;
					m_loopCount++;
				}
			}
			else
			{
				m_loopCount++;
			}
			m_durationMS = m_intervalms * m_loopCount;
		}
	});

	m_timerThread.reset(timerThread);
}

void VideoTimer::setPause(bool pause)
{
	m_pause = pause;
}

void VideoTimer::stop()
{
	m_stop = true;
	if(m_timerThread && m_timerThread->joinable()) m_timerThread->join();
	m_stop = false;
	m_pause = false;
	m_loopCount = 0;
}

double VideoTimer::timeMS()
{
	return m_durationMS;
}
