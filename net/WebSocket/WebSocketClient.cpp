#include "WebSocketClient.h"

void WebSocketClient::readHandle()
{
	std::string readStr(m_channel->readBuffer().data());
	printf("recv:\n%s\n", readStr.c_str());
	std::string webSocketKey;
	while (readStr.empty() == false) {
		std::string lineStr = readStr.substr(0, readStr.find("\n") - 1);
		if (lineStr.find("WebSocket-Key") != -1) {
			webSocketKey = lineStr.substr(lineStr.find("WebSocket-Key") + 15);
			break;
		}
		readStr = readStr.substr(readStr.find("\n") + 1);
	}

	std::string accept_key = WebSocketClient::generate_websocket_accept_key(webSocketKey);

	std::string response;
	response = response + "HTTP/1.1 101 Switching Protocols\r\n"
		+ "Upgrade: websocket\r\n"
		+ "Connection: Upgrade\r\n"
		+ "Sec-WebSocket-Accept: " + accept_key + "\r\n"
		+ "\r\n";

	m_channel->writeByteBuffer().append(response.c_str(), response.length());
}

std::vector<uint8_t> WebSocketClient::sha1(const std::string& input)
{
	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xEFCDAB89;
	uint32_t h2 = 0x98BADCFE;
	uint32_t h3 = 0x10325476;
	uint32_t h4 = 0xC3D2E1F0;

	size_t original_length = input.size() * 8;
	std::vector<uint8_t> data(input.begin(), input.end());
	data.push_back(0x80);

	while ((data.size() * 8) % 512 != 448) {
		data.push_back(0x00);
	}

	for (int i = 7; i >= 0; --i) {
		data.push_back(static_cast<uint8_t>((original_length >> (i * 8)) & 0xFF));
	}

	for (size_t i = 0; i < data.size(); i += 64) {
		uint32_t w[80];
		for (int j = 0; j < 16; ++j) {
			w[j] = (data[i + 4 * j] << 24) | (data[i + 4 * j + 1] << 16) |
				(data[i + 4 * j + 2] << 8) | data[i + 4 * j + 3];
		}
		for (int j = 16; j < 80; ++j) {
			w[j] = (w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16]);
			w[j] = (w[j] << 1) | (w[j] >> 31);
		}

		uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

		for (int j = 0; j < 80; ++j) {
			uint32_t f, k;
			if (j < 20) {
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if (j < 40) {
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if (j < 60) {
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else {
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}
			uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[j];
			e = d;
			d = c;
			c = (b << 30) | (b >> 2);
			b = a;
			a = temp;
		}

		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}

	std::vector<uint8_t> hash(20);
	for (int i = 0; i < 4; ++i) {
		hash[i] = (h0 >> (24 - i * 8)) & 0xFF;
		hash[i + 4] = (h1 >> (24 - i * 8)) & 0xFF;
		hash[i + 8] = (h2 >> (24 - i * 8)) & 0xFF;
		hash[i + 12] = (h3 >> (24 - i * 8)) & 0xFF;
		hash[i + 16] = (h4 >> (24 - i * 8)) & 0xFF;
	}
	return hash;
}

std::string WebSocketClient::base64_encode(const std::vector<uint8_t>& data)
{
	static const char* base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	std::string encoded;
	int val = 0, valb = -6;
	for (uint8_t c : data) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
	while (encoded.size() % 4) encoded.push_back('=');
	return encoded;
}

std::string WebSocketClient::generate_websocket_accept_key(const std::string& sec_websocket_key)
{
	std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	std::string key = sec_websocket_key + guid;

	// Compute SHA-1 hash
	std::vector<uint8_t> sha1_hash = sha1(key);

	// Base64 encode the hash
	return base64_encode(sha1_hash);
}

std::string WebSocketClient::construct_websocket_handshake_response(const std::string& sec_websocket_key)
{
	std::string accept_key = generate_websocket_accept_key(sec_websocket_key);

	std::ostringstream response;
	response << "HTTP/1.1 101 Switching Protocols\r\n"
		<< "Upgrade: websocket\r\n"
		<< "Connection: Upgrade\r\n"
		<< "Sec-WebSocket-Accept: " << accept_key << "\r\n"
		<< "\r\n";

	return response.str();
}