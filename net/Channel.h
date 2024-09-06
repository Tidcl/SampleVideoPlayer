#pragma once

#include <fstream>
#include "Comman.h"
#include "Handle.h"
#include "ByteBuffer.h"
#include "Poller.h"

class Poller;
class Handle;

class Channel : public std::enable_shared_from_this<Channel> {	//作为发送和接受的对象，把自己注册到事件循环中
public:
	Channel() {
		m_writeByteBuffer.init(1024);
		m_readByteBuffer.init(4096);
	};
	~Channel() { };

	SOCKET fd();
	void handle();
	void closeHandle();
	void setHandle(std::shared_ptr<Handle> handle);
	void setFD(SOCKET fd);
	void setPoller(std::shared_ptr<Poller> poller);

	ByteBuffer& writeByteBuffer() { return m_writeByteBuffer; };
	ByteBuffer& readBuffer() { return m_readByteBuffer; };

	void updateEventType();	//根据buffer中的内容更新关注的事件
	void setEventType(TEventType et);
	TEventType eventType();

	void setWriteFile(std::ifstream* ifs) { m_writeByteBuffer.setIfstream(ifs); };
private:
	std::shared_ptr<Poller> m_poller;	//用于更新自己的事件标志 以Channel的身份参与事件循环
	SOCKET m_fd = -1;
	ByteBuffer m_readByteBuffer;
	ByteBuffer m_writeByteBuffer;
	std::shared_ptr<Handle> m_handle;
	TEventType m_focusEventType = TEventType::none;	//如果写缓存有数据，那么关注write事件，当write事件触发就将数据写出去。如果写缓存为空，那么关于read事件，当read事件触发就从fd读取数据，并交给handle处理。
};
