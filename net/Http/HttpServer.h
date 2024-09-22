#pragma once

#include "TcpServer.h"
#include "HttpClientSession.h"

class HttpServer : public TcpServer {
public:
	HttpServer(std::shared_ptr<Poller> poller) : TcpServer(poller) {};
	~HttpServer() {};

	virtual void acceptHandle() override
	{
		SOCKET fd = m_acceptChannel->fd();
		sockaddr addr;
		int slen = sizeof(addr);
		SOCKET clientFd = accept(fd, &addr, &slen);
		if (clientFd > 0)
		{
			std::shared_ptr<HttpClientSession> tcpClient = std::make_shared<HttpClientHandle>();
			std::shared_ptr<Channel> clientChannel = std::make_shared<Channel>();
			clientChannel->setFD(clientFd);
			tcpClient->setChannel(clientChannel);
			clientChannel->setPoller(m_poller);
		}
	}
};