#pragma once

#include "Comman.h"
#include "Handle.h"
#include "Channel.h"
#include "Poller.h"
#include "TcpClient.h"

class TcpServer : public Handle, public std::enable_shared_from_this<TcpServer> {
public:
	TcpServer(std::shared_ptr<Poller> poller) : m_poller(poller) {
		m_acceptChannel = std::make_shared<Channel>();
		m_acceptChannel->setEventType(TEventType::acceptEvent);
	};
	~TcpServer() = default;

	void listen(std::shared_ptr<Poller> poller, int port = 8890);

	virtual void acceptHandle();

private:
	std::shared_ptr<Poller> m_poller;
	std::shared_ptr<Channel> m_acceptChannel;
	std::string m_ip;
	int m_port;
};


void TcpServer::listen(std::shared_ptr<Poller> poller, int port)
{
	int listen_fd;
	struct sockaddr_in addr;

	// 1. 创建一个TCP套接字
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// 2. 配置服务器地址结构体
	addr.sin_family = AF_INET; // IPv4
	addr.sin_addr.s_addr = INADDR_ANY; // 绑定到所有可用的接口
	addr.sin_port = htons(port); // 设置端口号

	// 3. 绑定套接字到指定的IP地址和端口
	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind failed");
		//close(listen_fd);
		exit(EXIT_FAILURE);
	}

	// 4. 将套接字置于监听状态
	if (::listen(listen_fd, 5) == -1) {
		perror("listen failed");
		//close(listen_fd);
		exit(EXIT_FAILURE);
	}

	m_acceptChannel->setFD(listen_fd);
	m_acceptChannel->setHandle(this->shared_from_this());
	//m_poller->addFD(listen_fd, m_acceptChannel);
	m_acceptChannel->setPoller(m_poller);


	//m_acceptor = std::make_shared<TcpAcceptor>(poller);
	//m_acceptor->init(port);
}

void TcpServer::acceptHandle()
{
	int fd = m_acceptChannel->fd();
	sockaddr addr;
	int slen = sizeof(addr);
	int clientFd = accept(fd, &addr, &slen);
	if (clientFd > 0)
	{
		std::shared_ptr<TcpClient> tcpClient = std::make_shared<TcpClient>();
		std::shared_ptr<Channel> clientChannel = std::make_shared<Channel>();
		clientChannel->setFD(clientFd);
		tcpClient->setChannel(clientChannel); 
		clientChannel->setPoller(m_poller);
	}
}
