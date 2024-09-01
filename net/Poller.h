#pragma once

#include "Channel.h"

class Channel;

class Poller {	//适配select、poll、epoll、iocp
public:
	Poller() = default;
	~Poller() = default;

	virtual void poll() {};

	void addFD(int fd, std::shared_ptr<Channel> channel) { 
		m_fdMap.insert({ fd, channel});
	};
	void rmFD(int fd) {
		m_fdMap.erase(fd);
	};
protected:
	std::map<int, std::shared_ptr<Channel>> m_fdMap;
};