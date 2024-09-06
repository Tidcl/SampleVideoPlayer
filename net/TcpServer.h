#pragma once

#include "Comman.h"
#include "Handle.h"
#include "Channel.h"
#include "Poller.h"

class TcpServer : public Handle, public std::enable_shared_from_this<TcpServer> {
public:
	TcpServer(std::shared_ptr<Poller> poller) : m_poller(poller) {
		m_acceptChannel = std::make_shared<Channel>();
		m_acceptChannel->setEventType(TEventType::acceptEvent);
	};
	~TcpServer() = default;

	void listen(int port = 8890);

protected:
	std::shared_ptr<Poller> m_poller;
	std::shared_ptr<Channel> m_acceptChannel;
	std::string m_ip;
	int m_port;
};
