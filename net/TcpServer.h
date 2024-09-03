#pragma once

#include "Comman.h"
#include "Handle.h"
#include "Channel.h"
#include "Poller.h"
#include "HttpClient.h"

class TcpServer : public Handle, public std::enable_shared_from_this<TcpServer> {
public:
	TcpServer(std::shared_ptr<Poller> poller) : m_poller(poller) {
		m_acceptChannel = std::make_shared<Channel>();
		m_acceptChannel->setEventType(TEventType::acceptEvent);
	};
	~TcpServer() = default;

	void listen(std::shared_ptr<Poller> poller, int port = 8890);

protected:
	std::shared_ptr<Poller> m_poller;
	std::shared_ptr<Channel> m_acceptChannel;
	std::string m_ip;
	int m_port;
};


void TcpServer::listen(std::shared_ptr<Poller> poller, int port)
{
	SOCKET listen_fd;
	struct sockaddr_in addr;

	// 1. ����һ��TCP�׽���
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// 2. ���÷�������ַ�ṹ��
	addr.sin_family = AF_INET; // IPv4
	addr.sin_addr.s_addr = INADDR_ANY; // �󶨵����п��õĽӿ�
	addr.sin_port = htons(port); // ���ö˿ں�

	// 3. ���׽��ֵ�ָ����IP��ַ�Ͷ˿�
	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind failed");
		//close(listen_fd);
		exit(EXIT_FAILURE);
	}

	// 4. ���׽������ڼ���״̬
	if (::listen(listen_fd, 5) == -1) {
		perror("listen failed");
		//close(listen_fd);
		exit(EXIT_FAILURE);
	}

	m_acceptChannel->setFD(listen_fd);
	m_acceptChannel->setHandle(this->shared_from_this());
	m_acceptChannel->setPoller(m_poller);
}

