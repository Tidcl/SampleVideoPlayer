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

	virtual void readHandle() override;

	// Function to perform a SHA-1 hash
	static std::vector<uint8_t> sha1(const std::string& input);

	// Function to perform Base64 encoding
	static std::string base64_encode(const std::vector<uint8_t>& data);

	// Function to generate WebSocket accept key
	static std::string generate_websocket_accept_key(const std::string& sec_websocket_key);

	// Function to construct the WebSocket handshake response
	static std::string construct_websocket_handshake_response(const std::string& sec_websocket_key);
};