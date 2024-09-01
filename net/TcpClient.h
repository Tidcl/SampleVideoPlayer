#pragma once
#include "Channel.h"
#include <memory>
#include "Poller.h"

class TcpClient : public Handle, public std::enable_shared_from_this<TcpClient>{	//�ڷ��������򴴽��ͻ��˶��󣬾��Ƿ���ͻ��˵�
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
		//�����ṩ�ķ������ͣ���װ����ͷ β
	};

	virtual void errorHandle() override
	{
	};
protected:
	std::shared_ptr<Channel> m_channel;

	friend class Acceptor;
};