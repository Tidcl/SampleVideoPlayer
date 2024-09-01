#pragma once
#include "Channel.h"
#include <memory>
#include "Poller.h"

class TcpClient : public Handle, public std::enable_shared_from_this<TcpClient>{	//在服务器程序创建客户端对象，就是服务客户端的
public:
	TcpClient() = default;
	~TcpClient() {
	};

	void setChannel(std::shared_ptr<Channel> channel){
		m_channel = channel;
		m_channel->setEventType(TEventType::readEvent);
		m_channel->setHandle(shared_from_this());
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
protected:
	std::shared_ptr<Channel> m_channel;

	friend class Acceptor;
};