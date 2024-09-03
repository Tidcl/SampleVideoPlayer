#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include "TcpClient.h"

class HttpClient : public TcpClient{
public:
	HttpClient() = default;
	~HttpClient() = default;

	enum REQTYPE {
		RESTFUL,
		FILE
	};

	void dealRestfulReq();

	void dealFileQeq();

	//void testDealWebSocket();

	virtual void readHandle() override;

	std::string getCurrentTimeInGMT();

	std::string getContentType(std::string& fileSuffix);

	bool fileExists(const std::string& filename);
private:
	std::string m_reqUrlPath;

	bool m_isChunk;

	std::map<std::string, std::ifstream*> m_chunkFile;
};