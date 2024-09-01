#pragma once
#include "Channel.h"
#include <memory>
#include "Poller.h"

class TcpClient : public Handle, public std::enable_shared_from_this<TcpClient>{	//在服务器程序创建客户端对象，就是服务客户端的
public:
	TcpClient() = default;
	~TcpClient() {
	};

	void setChannel(std::shared_ptr<Channel> channel){
		m_channel = channel;
		m_channel->setEventType(TEventType::readEvent);
		m_channel->setHandle(shared_from_this());
	};

	virtual void readHandle() override
	{
		printf("%s\n", m_channel->readData().c_str());
		m_channel->writeToBuffer(m_channel->readData());
		std::string str = R"(HTTP/1.1 200 OK
Date: Sat, 31 Aug 2024 12:00:00 GMT
Content-Type: text/html; charset=UTF-8
Content-Length: 143
Connection: close

<!DOCTYPE html>
<html>
<head>
<title>Page Title</title>
</head>
<body>

<h1>This is a Heading</h1>
<p>This is a paragraph.</p>

</body>
</html>)";
		m_channel->writeToBuffer(str);


	};

	virtual void writeHandle() override
	{
		//根据提供的服务类型，组装返回头 尾
	};

	virtual void errorHandle() override
	{
	};
private:
	std::shared_ptr<Channel> m_channel;

	friend class Acceptor;
};