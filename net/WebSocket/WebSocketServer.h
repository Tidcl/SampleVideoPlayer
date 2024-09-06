#pragma once

#include "TcpServer.h"
#include "WebSocketClientHandle.h"

class WebSocketServer : public TcpServer {
public:
	WebSocketServer(std::shared_ptr<Poller> poller) : TcpServer(poller) {};
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
		std::shared_ptr<WebSocketClientHandle> tcpClient = std::make_shared<WebSocketClientHandle>();
		std::shared_ptr<Channel> clientChannel = std::make_shared<Channel>();
		clientChannel->setFD(clientFd);

		tcpClient->setChannel(clientChannel);
		clientChannel->setPoller(m_poller);
	}
}
