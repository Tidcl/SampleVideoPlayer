#include "TcpServer.h"

void TcpServer::listen(int port /*= 8890*/)
{
	SOCKET listen_fd;
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
	m_acceptChannel->setPoller(m_poller);
}

