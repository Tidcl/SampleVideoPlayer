#include "SelectPoller.h"

//poll目前只关注了网络 可以利用循环加入定时任务或者一些延时任务处理
void SelectPoller::poll()
{
	//清空fd_set
	FD_ZERO(&m_readSet);
	FD_ZERO(&m_writeSet);
	FD_ZERO(&m_errorSet);

	auto fdMap = this->m_fdMap;

	int fdCount = 0;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100;
	for (auto mapItem : fdMap)
	{
		fdCount++;
		FDEventType et = mapItem.second->eventType();
		switch (et)
		{
		case none:
			break;
		case acceptEvent:
			FD_SET(mapItem.first, &m_readSet);
			break;
		case readEvent:
			FD_SET(mapItem.first, &m_readSet);
			break;
		case writeEvent:
			FD_SET(mapItem.first, &m_writeSet);
			break;
		case errorEvent:
			break;
		default:
			break;
		}
	}

	//重新设置fd到各种set
	int activeFdCount = ::select(fdCount, &m_readSet, &m_writeSet, &m_errorSet, &timeout);

	//读事件
	bool isActive = false;
	for (int i = 0; i < activeFdCount; i++)
	{
		bool isFinish = false;
		for (auto mapItem : fdMap)
		{
			if (isFinish == false)
			{
				SOCKET fd = mapItem.first;
				if (FD_ISSET(fd, &m_readSet) | FD_ISSET(fd, &m_writeSet) | FD_ISSET(fd, &m_errorSet))
				{
					isFinish = true;
					isActive = true;
					std::shared_ptr<Channel> channel = mapItem.second;
					//go co_scheduler(co_sched) [channel]() {
						channel->handle();
					//};
				}
			}
		}
	}

	if(isActive) co_yield;
}
