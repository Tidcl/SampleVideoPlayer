#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include "TcpClient.h"

class HttpClient : public TcpClient{
public:
	HttpClient() = default;
	~HttpClient() = default;

	virtual void readHandle() override {
		//printf("%s\n", m_channel->readData().c_str());
		std::string& readStr = m_channel->readData();
		size_t startIndex = readStr.find_first_of(" ");
		size_t endIndex = readStr.find_first_of(" ", startIndex + 1);
	 	std::string url = readStr.substr(startIndex + 2, endIndex - startIndex - 2);
		m_channel->writeToBuffer(url);

		//HTTP / 1.1 200 OK
		//	Date : Sat, 31 Aug 2024 12 : 00 : 00 GMT
		//	Content - Type : text / html; charset = UTF - 8
		//	Content - Length: 143
		//	Connection : close
//		std::string str = R"(<!DOCTYPE html>
//<html>
//<head>
//<title>Page Title</title>
//</head>
//<body>
//
//<h1>This is a Heading</h1>
//<p>This is a paragraph.</p>
//
//</body>
//</html>)";
//		m_channel->writeToBuffer(str);
//

	};

	virtual void writeHandle() override {
		char strbuf[1024];
		std::string response;
		std::vector<std::string> strVec;
		strVec.emplace_back(std::move(std::string("HTTP/1.1 200 OK")));
		strVec.emplace_back("Date: " + getCurrentTimeInGMT());
		strVec.emplace_back(std::move(std::string("Content-Type: text/html; charset=UTF-8")));

		std::string str = R"(
<!DOCTYPE html>
<html>
<head>
<title>Page Title</title>
</head>
<body>

%s

</body>
</html>)";
		str.replace(str.find("%s"), 2, m_channel->writeBuffer());
		memset(strbuf, 0, 1024);
		sprintf(strbuf, "Content-Length: %d ", str.length());
		strVec.emplace_back(std::move(std::string(strbuf)));
		strVec.emplace_back(std::move(str));

		for (auto& str : strVec)
		{
			response += (str + "\r\n");
		}
		//printf("\n\n%s\n", response.c_str());
		m_channel->writeBuffer() = response;
	};

	virtual void errorHandle() override {
	};

	std::string getCurrentTimeInGMT() {
		// 获取当前时间并转换为时间结构
		std::time_t now = std::time(nullptr);
		// 格式化时间为"Sat, 31 Aug 2024 12:00:00 GMT"
		char buffer[100];
		strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));

		return std::string(buffer);
	}
};