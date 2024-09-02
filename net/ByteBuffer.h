#pragma once

#include "Comman.h"

class ByteBuffer {
public:
	ByteBuffer() = default;
	ByteBuffer(ByteBuffer& other) {
		this->init(other.m_capacity);
		memcpy_s(m_buffer, m_capacity, other.m_buffer, other.m_capacity);
		this->m_capacity = other.m_capacity;
		this->m_readPos = other.m_readPos;
		this->m_writePos = other.m_writePos;
	}

	ByteBuffer(ByteBuffer&& other) {
		this->m_buffer = other.m_buffer;
		other.m_buffer = nullptr;
		this->m_capacity = other.m_capacity;
		other.m_capacity = 0;
		this->m_readPos = other.m_readPos;
		other.m_readPos = 0;
		this->m_writePos = other.m_writePos;
		other.m_writePos = 0;
	}

	ByteBuffer& operator=(const ByteBuffer& other) {
		this->init(other.m_capacity);
		memcpy_s(m_buffer, m_capacity, other.m_buffer, other.m_capacity);
		this->m_capacity = other.m_capacity;
		this->m_readPos = other.m_readPos;
		this->m_writePos = other.m_writePos;
		return *this;
	}

	ByteBuffer& operator=(ByteBuffer&& other) {
		this->m_buffer = other.m_buffer;
		other.m_buffer = nullptr;
		this->m_capacity = other.m_capacity;
		other.m_capacity = 0;
		this->m_readPos = other.m_readPos;
		other.m_readPos = 0;
		this->m_writePos = other.m_writePos;
		other.m_writePos = 0;
		return *this;
	}

	~ByteBuffer() {
		if (m_buffer) {
			delete[] m_buffer;
			m_buffer = nullptr;
		}
	};

	void init(int size) {
		if (m_buffer) delete[] m_buffer;
		m_buffer = new char[size];
		memset(m_buffer, 0, size);
		m_capacity = size;
		m_readPos = 0;
		m_writePos = 0;
	};

	bool append(const char* data, int size) {
		if (m_writePos + size > m_capacity)
		{
			int newOpacity = m_capacity * 1.5;
			while (newOpacity < m_writePos + size)
			{
				newOpacity = newOpacity * 1.5;
			}
			char* newBuffer = new char[newOpacity];
			memset(newBuffer, 0, newOpacity);
			memcpy_s(newBuffer, newOpacity, m_buffer, m_capacity);
			delete[] m_buffer;
			m_buffer = newBuffer;
			m_capacity = newOpacity;
		}

		int rtn = memcpy_s(m_buffer + m_writePos, m_capacity - m_writePos, data, size);
		m_writePos += size;

		return true;
	};

	char* data() {
		return m_buffer;
	};

	char* take(int length) {
		if (m_readPos == m_writePos) {
			return nullptr;
		}

		char* readPos = m_buffer + m_readPos;
		
		if (length + m_readPos > m_writePos){
			length = m_writePos - m_readPos;
		}
		char* d = new char[length];
		memcpy_s(d, length, readPos, length);
		m_readPos += length;
		return d;
	}

	void clear() {
		memset(m_buffer, 0 ,m_capacity);
		m_writePos = 0;
		m_readPos = 0;
	}

	int length() {
		return m_writePos;
	};

	int capacity() {
		return m_capacity;
	}

	bool empty() {
		return m_writePos == 0 ? true : false;
	};
private:
	char* m_buffer = nullptr;
	int m_capacity = 0;
	int m_writePos = 0;
	int m_readPos = 0;
};
