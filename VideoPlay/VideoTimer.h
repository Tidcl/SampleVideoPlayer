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

	void setInterval(double intervalms);		//���ö�ʱ��ʱ���� ����
	void setCallbackFun(VideoTimerFun func, void* value);//���ö�ʱ���Ļص�����
	void start();							//������ʱ��
	void setPause(bool pause);							//��ͣ��ʱ��
	void stop();							//�رն�ʱ��
	double timeMS();							//��ǰʱ��
private:
	double m_intervalms = -1;						//���ü��ʱ��ms ����
	bool m_pause = false;							//��ʱ����ͣ��־λ
	int m_loopCount = 0;						//��ʱ��ѭ�����������interval���㵱ǰ��ʱ�����е�ʱ��
	bool m_stop = false;							//�Ƿ�ֹͣ��ʱ��

	std::function<void(void)> m_ffun;
	VideoTimerFun m_func = nullptr;
	std::shared_ptr<std::thread> m_timerThread = nullptr;				//���ж�ʱ���߼����߳�
};