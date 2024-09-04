#include <coroutine.h>
#include "Channel.h"

void Channel::updateEventType()
{
	if (m_focusEventType == TEventType::acceptEvent)	//accept事件的channel不会切换事件，因此不需要改变
	{
		return;
	}

	if (m_writeByteBuffer.empty() == false)
	{
		m_focusEventType = TEventType::writeEvent;
	}
	else
	{
		m_focusEventType = TEventType::readEvent;
	}
}


void Channel::setEventType(TEventType et)
{
	m_focusEventType = et;
}

TEventType Channel::eventType()
{
	updateEventType();
	return m_focusEventType;
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
	if (eventType() == TEventType::acceptEvent)
	{
		m_handle->acceptHandle();
	}
	else if (eventType() == TEventType::readEvent)
	{
		m_readByteBuffer.clear();
		char* bufferAddr = const_cast<char*>(m_readByteBuffer.data());
		int rtn = recv(m_fd, bufferAddr, m_readByteBuffer.capacity(), 0);
		if (rtn > 0)
		{
			m_readByteBuffer.m_writePos = rtn;
			//go std::bind(&Handle::readHandle, m_handle);

			m_handle->readHandle();
		}
		else
		{
			m_poller->rmFD(m_fd);
			closesocket(m_fd);
			m_handle.reset();	//释放掉handler
		}

	}
	else if (eventType() == TEventType::writeEvent)
	{
		//m_handle->writeHandle();
		//m_writeByteBuffer;
		int rtn = send(m_fd, m_writeByteBuffer.data(), m_writeByteBuffer.length(), 0);
		if (rtn == m_writeByteBuffer.length())
		{
			m_writeByteBuffer.clear();
			m_focusEventType = TEventType::readEvent;
		}
		else if (rtn == -1)
		{
			m_poller->rmFD(m_fd);
			closesocket(m_fd);
			m_handle.reset();	//释放掉handler
		}
		else
		{
			char* buffer = m_writeByteBuffer.take(rtn);
			delete[] buffer;
		}
	}

	//co_yield;
}
