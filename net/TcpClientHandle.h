#pragma once
#include "Channel.h"
#include <memory>
#include "Poller.h"

class TcpClientHandle : public Handle, public std::enable_shared_from_this<TcpClientHandle>{	//�ڷ��������򴴽��ͻ��˶��󣬾��Ƿ���ͻ��˵�
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
		//�����ṩ�ķ������ͣ���װ����ͷ β
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