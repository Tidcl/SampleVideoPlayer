#pragma once

#include <thread>
#include <chrono>
#include <functional>
#include <iostream>

typedef void(*VideoTimerFun)(void*);

class VideoTimer {
public:
	VideoTimer();
	~VideoTimer();

	void setInterval(double intervalms);		//设置定时器时间间隔 毫秒
	void setCallbackFun(VideoTimerFun func, void* value);//设置定时器的回调函数
	void start();							//开启定时器
	void setPause(bool pause);							//暂停定时器
	void stop();							//关闭定时器
	double timeMS();							//当前时刻
private:
	double m_intervalms = -1;						//设置间隔时间ms 毫秒
	bool m_pause = false;							//定时器暂停标志位
	int m_loopCount = 0;						//定时器循环次数，配合interval计算当前定时器运行的时刻
	bool m_stop = false;							//是否停止定时器

	std::function<void(void)> m_ffun;
	VideoTimerFun m_func = nullptr;
	std::shared_ptr<std::thread> m_timerThread = nullptr;				//运行定时器逻辑的线程
};