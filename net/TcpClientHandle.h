#pragma once
#include "Channel.h"
#include <memory>
#include "Poller.h"

class TcpClientHandle : public Handle, public std::enable_shared_from_this<TcpClientHandle>{	//在服务器程序创建客户端对象，就是服务客户端的
public:
	TcpClientHandle() = default;
	~TcpClientHandle() {
	};

	void setChannel(std::shared_ptr<Channel> channel) {
		m_channel = channel;
		m_channel->setEventType(FDEventType::readEvent);
	};

	virtual void readHandle() override
	{
	};

	virtual void writeHandle() override
	{
		//根据提供的服务类型，组装返回头 尾
	};

	virtual void errorHandle() override
	{
	};

	virtual void closeHandle() override 
	{
	}
protected:
	std::shared_ptr<Channel> m_channel;

	friend class Acceptor;
};