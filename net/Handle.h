#pragma once

#include "Comman.h"
#include "Channel.h"

class Channel;

class Handle {	//让每个需要具体处理的类进行实现，比如accptor、tcpServer、tcpClient
public:
	Handle() = default;
	~Handle() = default;

	virtual void acceptHandle() {};
	virtual void readHandle() {};
	virtual void writeHandle() {};
	virtual void errorHandle() {};

	std::shared_ptr<Channel> m_connect;
};