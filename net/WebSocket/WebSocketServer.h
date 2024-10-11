#pragma once

#include "TcpServer.h"
#include "WebSocketClientSession.h"

class WebSocketServer : public TcpServer {
public:
	WebSocketServer(std::shared_ptr<Poller> poller) : TcpServer(poller) {
		;
	};
	~WebSocketServer() {};

	virtual void acceptHandle() override;
};

void WebSocketServer::acceptHandle()
{
	SOCKET fd = m_acceptChannel->fd();
	sockaddr addr;
	int slen = sizeof(addr);
	SOCKET clientFd = accept(fd, &addr, &slen);
	if (clientFd > 0)
	{
		std::shared_ptr<WebSocketClientSession> tcpClient = std::make_shared<WebSocketClientSession>();
		std::shared_ptr<Channel> clientChannel = std::make_shared<Channel>();
		tcpClient->setChannel(clientChannel);
		clientChannel->setFD(clientFd);
		clientChannel->setHandle(tcpClient);
		clientChannel->setEventType(FDEventType::readEvent);
		clientChannel->addSelfToPoller(m_poller);
	}
}
