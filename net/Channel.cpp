#include <coroutine.h>
#include "Channel.h"

void Channel::updateEventType()
{
	if (m_focusEventType == FDEventType::acceptEvent)	//accept事件的channel不会切换事件，因此不需要改变
	{
		return;
	}

	if (m_writeByteBuffer.empty() == false)
	{
		m_focusEventType = FDEventType::writeEvent;
	}
	else
	{
		m_focusEventType = FDEventType::readEvent;
	}
}


void Channel::setEventType(FDEventType et)
{
	m_focusEventType = et;
}

FDEventType Channel::eventType()
{
	updateEventType();
	return m_focusEventType;
}

void Channel::setWriteFile(std::ifstream* ifs)
{
	m_writeByteBuffer.setIfstream(ifs);
}

void Channel::setHandle(std::shared_ptr<Handle> handle)
{
	m_handle = handle;
}

void Channel::setFD(SOCKET fd)
{
	m_fd = fd;
}

void Channel::setPoller(std::shared_ptr<Poller> poller)
{
	m_poller = poller;
	m_poller->addFD(m_fd, shared_from_this());
}

SOCKET Channel::fd()
{
	return m_fd;
}

void Channel::handle()
{
	if (eventType() == FDEventType::acceptEvent)
	{
		m_handle->acceptHandle();
	}
	else if (eventType() == FDEventType::readEvent)
	{
		m_readByteBuffer.clear();
		char* bufferAddr = const_cast<char*>(m_readByteBuffer.data());
		int rtn = recv(m_fd, bufferAddr, m_readByteBuffer.capacity(), 0);
		if (rtn > 0)
		{
			m_readByteBuffer.m_writePos = rtn;

			m_handle->readHandle();
		}
		else
		{
			closeHandle();
		}

	}
	else if (eventType() == FDEventType::writeEvent)
	{
		int rtn = send(m_fd, m_writeByteBuffer.data(), m_writeByteBuffer.length(), 0);
		if (rtn == m_writeByteBuffer.length())
		{
			m_writeByteBuffer.clear();
			m_focusEventType = FDEventType::readEvent;
		}
		else if (rtn == -1)
		{
			closeHandle();
		}
		else
		{
			char* buffer = m_writeByteBuffer.take(rtn);
			delete[] buffer;
		}
	}
}

void Channel::closeHandle()
{
	m_poller->rmFD(m_fd);
	closesocket(m_fd);
	m_handle->closeHandle();
	m_handle.reset();	//释放掉handler
}
