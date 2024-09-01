#pragma once

#include "Comman.h"

class ByteBuffer {
public:
	ByteBuffer() = default;
	~ByteBuffer() = default;

	int length();
	void appendBytes(char* buffer, int size);
	char* takeBytes(int size);

	bool isEmpty();
private:
	char* m_buffer = nullptr;
	int m_length = 0;

	int m_writePos = 0;
	int m_readPos = 0;
};
