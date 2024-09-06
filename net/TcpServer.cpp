#include "TcpServer.h"

void TcpServer::listen(int port /*= 8890*/)
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

