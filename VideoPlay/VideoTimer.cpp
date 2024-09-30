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
		double leftTimeUs = 0;
		double tempLoopCount = 0;
		long long waiteTime = 0;

		while (m_stop == false)
		{
			while (m_pause)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			start = std::chrono::high_resolution_clock::now();
			if (m_func) m_ffun();

			duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
			leftTimeUs = m_intervalms * 1000 - duration;	//计算剩余的微妙
			tempLoopCount = (duration / (m_intervalms * 1000));
			if (leftTimeUs > 0)	//还需要等待
			{
				std::this_thread::sleep_for(std::chrono::microseconds((long)leftTimeUs));
				m_loopCount++;
			}
			else //多出来的时间
			{
				waiteTime = (leftTimeUs)+(tempLoopCount * m_intervalms * 1000);
				std::this_thread::sleep_for(std::chrono::microseconds(waiteTime));
				//std::cout << "us   m_interval " << m_intervalms * 1000 << "    duration " << duration << "     waiteTime " << waiteTime << "    tempLoopCount " << tempLoopCount << std::endl;
				m_loopCount += (tempLoopCount + 1);
			}
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
	return m_intervalms * m_loopCount;
}
