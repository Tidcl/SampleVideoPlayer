#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include "TcpClientHandle.h"

class HttpClientHandle : public TcpClientHandle{
public:
	HttpClientHandle() = default;
	~HttpClientHandle() = default;

	virtual void readHandle() override;

	enum REQTYPE {
		RESTFUL,
		FILE
	};

	void dealRestfulReq();

	void dealFileQeq();

	//void testDealWebSocket();

	std::string getCurrentTimeInGMT();

	std::string getContentType(std::string& fileSuffix);

	bool fileExists(const std::string& filename);
private:
	std::string m_reqUrlPath;

	bool m_isChunk;

	std::map<std::string, std::ifstream*> m_chunkFile;
};