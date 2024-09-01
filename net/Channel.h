#pragma once

#include "Comman.h"
#include "Handle.h"
#include "ByteBuffer.h"
#include "Poller.h"

class Poller;
class Handle;

class Channel : public std::enable_shared_from_this<Channel> {	//作为发送和接受的对象，把自己注册到事件循环中
public:
	Channel() { m_readBuffer.resize(4096); m_writeBuffer.clear(); };
	~Channel() { };

	int fd();
	void handle();
	void setHandle(std::shared_ptr<Handle> handle);
	void setFD(int fd);
	void setPoller(std::shared_ptr<Poller> poller);

	std::string& writeBuffer() { return m_writeBuffer; };
	std::string& readData() { return m_readData; };
	std::string& readBuffer() { return m_readBuffer; };
	int writeToBuffer(std::string str);		//把m_writeBuffer中的数据写出去
	int readToBuffer(std::string& str);		//读取到m_readBuffer中

	void updateEventType();	//根据buffer中的内容更新关注的事件
	void setEventType(TEventType et);
	TEventType eventType();

	
private:
	std::shared_ptr<Poller> m_poller;	//用于更新自己的事件标志 以Channel的身份参与事件循环
	int m_fd;
	std::string m_readBuffer;
	std::string m_readData;
	std::string m_writeBuffer;
	std::shared_ptr<Handle> m_handle;
	TEventType m_focusEventType;	//如果写缓存有数据，那么关注write事件，当write事件触发就将数据写出去。如果写缓存为空，那么关于read事件，当read事件触发就从fd读取数据，并交给handle处理。
};
