#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include "TcpClient.h"
class WebSocketClient : public TcpClient {
public:
	WebSocketClient() {};
	~WebSocketClient() {};

	enum FrameType {


	};

	virtual void readHandle() override;

	void dealData(const char* buffer);

	bool isShakeHands(const char* buffer);

	bool deal1Frame();	//是否最后一帧

	bool deal2Frame(const char* buffer);	//是否被掩码处理过

	char* newDataFromBuffer(const char* buffer);

	void dealShakeHands(const char* buffer);

	void getLengthBytes(size_t contentLength, char*& byte, int& length);
	

	// Function to perform a SHA-1 hash
	static std::vector<uint8_t> sha1(const std::string& input);

	// Function to perform Base64 encoding
	static std::string base64_encode(const std::vector<uint8_t>& data);

	// Function to generate WebSocket accept key
	static std::string generate_websocket_accept_key(const std::string& sec_websocket_key);

	// Function to construct the WebSocket handshake response
	static std::string construct_websocket_handshake_response(const std::string& sec_websocket_key);
};