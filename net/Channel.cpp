#include "Channel.h"


void Channel::updateEventType()
{
	if (m_focusEventType == TEventType::acceptEvent)	//accept事件的channel不会切换事件，因此不需要改变
	{
		return;
	}

	if (m_writeBuffer.empty() == false)
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

void Channel::setFD(int fd)
{
	m_fd = fd;
}

void Channel::setPoller(std::shared_ptr<Poller> poller)
{
	m_poller = poller;
	m_poller->addFD(m_fd, shared_from_this());
}

int Channel::fd()
{
	return m_fd;
}

int Channel::writeToBuffer(std::string str)
{
	m_writeBuffer = str;
	return 0;
}

int Channel::readToBuffer(std::string& str)
{
	str = m_readBuffer;
	return 0;
}

void Channel::handle()
{
	if (eventType() == TEventType::acceptEvent)
	{
		m_handle->acceptHandle();
	}
	else if (eventType() == TEventType::readEvent)
	{
		char* bufferAddr = const_cast<char*>(m_readBuffer.c_str());
		int rtn = recv(m_fd, bufferAddr, m_readBuffer.length(), 0);
		if (rtn > 0)
		{
			m_readData = m_readBuffer.substr(0, rtn);
			m_handle->readHandle();
		}
		else
		{
			m_poller->rmFD(m_fd);
			m_handle.reset();	//释放掉handler
		}

	}
	else if (eventType() == TEventType::writeEvent)
	{
		m_handle->writeHandle();

		std::string& wBuffer = m_writeBuffer;
		int rtn = send(m_fd, wBuffer.c_str(), wBuffer.length(), 0);
		if (rtn == wBuffer.length())
		{
			wBuffer.clear();
		}
		else
		{
			wBuffer = wBuffer.substr(rtn);
		}
	}
}
