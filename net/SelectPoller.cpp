#include "SelectPoller.h"
//
//#pragma comment(lib, "Ws2_32.lib")
//#pragma comment(lib,"Iphlpapi.lib")

void SelectPoller::poll()
{
	//清空fd_set
	FD_ZERO(&m_readSet);
	FD_ZERO(&m_writeSet);
	FD_ZERO(&m_errorSet);

	auto fdMap = this->m_fdMap;

	int fdCount = 0;
	for (auto mapItem : fdMap)
	{
		fdCount++;
		TEventType et = mapItem.second->eventType();
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
	int activeFdCount = ::select(fdCount, &m_readSet, &m_writeSet, &m_errorSet, nullptr);

	//读事件
	for (int i = 0; i < activeFdCount; i++)
	{
		bool isFinish = false;
		for (auto mapItem : fdMap)
		{
			if (isFinish == false)
			{
				SOCKET fd = mapItem.first;
				std::shared_ptr<Channel> channel = mapItem.second;

				if (FD_ISSET(fd, &m_readSet) | FD_ISSET(fd, &m_writeSet) | FD_ISSET(fd, &m_errorSet))
				{
					channel->handle();
					isFinish = true;
				}
			}
		}
	}

	////检查fd是否触发事件
	//FD_ISSET(fd, &m_readSet);	//调用readHandle
	//FD_ISSET(fd, &m_writeSet);	//调用writeHandle
	//FD_ISSET(fd, &m_errorSet);	//调用errorHandle
}
