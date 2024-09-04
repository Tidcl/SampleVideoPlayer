#include "HttpClient.h"
#include "WebSocketClient.h"

void HttpClient::readHandle()
{
	//testDealWebSocket();
	std::string readStr(m_channel->readBuffer().data());
	size_t startIndex = readStr.find_first_of(" ");
	size_t endIndex = readStr.find_first_of(" ", startIndex + 1);
	m_reqUrlPath = readStr.substr(startIndex + 2, endIndex - startIndex - 2);

	//是否能在当前目录中找到html文件
	int suffixIndex = (int)m_reqUrlPath.find(".");
	if (suffixIndex == -1)
	{
		if (fileExists(m_reqUrlPath + ".html")) {
			m_reqUrlPath += ".html";
			dealFileQeq();
		}
		else {
			dealRestfulReq();
		}
	}
	else
	{
		dealFileQeq();
	}
}

void HttpClient::dealRestfulReq()
{
	char strbuf[1024];
	ByteBuffer response;
	response.init(4096);
	std::vector<std::string> strVec;
	strVec.emplace_back(std::move(std::string("HTTP/1.1 200 OK")));
	strVec.emplace_back("Date: " + getCurrentTimeInGMT());
	strVec.emplace_back(std::move(std::string("Content-Type: text/html; charset=UTF-8")));
	std::string content;


	memset(strbuf, 0, 1024);
	sprintf(strbuf, "Content-Length: %d \r\n", (int)content.length());
	strVec.emplace_back(std::move(std::string(strbuf)));
	strVec.emplace_back(std::move(content));
	//组装成应答包
	for (auto& str : strVec)
	{
		response.append(str.c_str(), str.length());
		response.append("\r\n", strlen("\r\n"));
	}
	response.append("\r\n", strlen("\r\n"));
	response.append(content.data(), content.length());
	m_channel->writeByteBuffer() = response;
}

void HttpClient::dealFileQeq()
{
	std::string fileSuffix = m_reqUrlPath.substr(m_reqUrlPath.find(".") + 1);
	char strbuf[1024];
	ByteBuffer response;
	response.init(4096);
	std::vector<std::string> strVec;
	strVec.emplace_back(std::move(std::string("HTTP/1.1 200 OK")));
	strVec.emplace_back("Date: " + getCurrentTimeInGMT());
	memset(strbuf, 0, 1024);
	sprintf(strbuf, "Content-Type: %s", getContentType(fileSuffix).c_str());
	strVec.emplace_back(strbuf);

	//std::string content;
	//text image audio video application multipart
	bool isFileReq = m_reqUrlPath.find(".") != -1 ? true : false;
	//默认只能读取运行目录的文件
	ByteBuffer errorContent;
	long fileLength = 0;
	{
		std::ifstream* ifs = new std::ifstream(m_reqUrlPath, std::ios::binary);
		if (!ifs->is_open())
		{
			std::string contentStr = std::string("not find file %s.");
			contentStr.replace(contentStr.find("%s"), 2, m_reqUrlPath);
			errorContent.append(contentStr.c_str(), contentStr.length());
			delete ifs;
		}
		else
		{
			ifs->seekg(0, std::ios::end);
			fileLength = (long)ifs->tellg();
			ifs->seekg(0, std::ios::beg);
			m_channel->setWriteFile(ifs);
			//m_channel->set
			////std::string line;
			//int readCount = 4096;
			//content.init(readCount);
			////int readCount = 0;
			//char buffer[4096];
			//memset(buffer, 0, readCount);
			//while (ifs.read(buffer, 4096) || ifs.gcount() > 0) {
			//	readCount = ifs.gcount();

			//	//content += buffer;
			//	content.append(buffer, readCount);
			//	memset(buffer, 0, readCount);
			//}
			//ifs.close();
		}
	}

	memset(strbuf, 0, 1024);
	//构造数据长度
	if (errorContent.empty()) {
		sprintf(strbuf, "Content-Length: %d", fileLength);
		strVec.emplace_back(std::move(std::string(strbuf)));
	}
	else
	{
		sprintf(strbuf, "Content-Length: %d", errorContent.length());
		strVec.emplace_back(std::move(std::string(strbuf)));
	}
	//组装成应答包
	for (auto& str : strVec)
	{
		response.append(str.c_str(), str.length());
		response.append("\r\n", strlen("\r\n"));
	}
	response.append("\r\n", strlen("\r\n"));
	if (errorContent.empty() == false) {
		response.append(errorContent.data(), errorContent.length());
	}

	m_channel->writeByteBuffer() = response;
}



//void HttpClient::testDealWebSocket()
//{
//	std::string readStr(m_channel->readBuffer().data());
//	printf("recv:\n%s\n", readStr.c_str());
//	std::string webSocketKey;
//	while (readStr.empty() == false) {
//		std::string lineStr = readStr.substr(0, readStr.find("\n")-1);
//		if (lineStr.find("WebSocket-Key") != -1) {
//			webSocketKey = lineStr.substr(lineStr.find("WebSocket-Key") + 15);
//			break;
//		}
//		readStr = readStr.substr(readStr.find("\n")+1);
//	}
//
//	std::string accept_key = WebSocketClient::generate_websocket_accept_key(webSocketKey);
//	
//	std::string response;
//	response = response + "HTTP/1.1 101 Switching Protocols\r\n"
//		+ "Upgrade: websocket\r\n"
//		+ "Connection: Upgrade\r\n"
//		+ "Sec-WebSocket-Accept: " + accept_key + "\r\n"
//		+ "\r\n";
//
//	m_channel->writeByteBuffer().append(response.c_str(), response.length());
//}

std::string HttpClient::getCurrentTimeInGMT()
{
	// 获取当前时间并转换为时间结构
	std::time_t now = std::time(nullptr);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
	
	return std::string(buffer);
}

std::string HttpClient::getContentType(std::string& fileSuffix)
{
	if (fileSuffix == "html" || fileSuffix == "plain")
	{
		return "text/" + fileSuffix;
	}

	if (fileSuffix == "ico" || fileSuffix == "gif" || fileSuffix == "jpeg" || fileSuffix == "png" || fileSuffix == "webp")
	{
		return "image/" + fileSuffix;
	}

	if (fileSuffix == "mpeg" || fileSuffix == "wav" || fileSuffix == "ogg")
	{
		return "audio/" + fileSuffix;
	}

	if (fileSuffix == "mp4" || fileSuffix == "webm" || fileSuffix == "ogg")
	{
		return "video/" + fileSuffix;
	}

	if (fileSuffix == "json" || fileSuffix == "xml" || fileSuffix == "octet-stream")
	{
		return "application/" + fileSuffix;
	}

	return "application/" + fileSuffix;
}

bool HttpClient::fileExists(const std::string& filename)
{
	std::ifstream file(filename);
	return file.good();
}