#pragma once

#include "Comman.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>

class ByteBuffer {
public:
	ByteBuffer() = default;
	ByteBuffer(ByteBuffer& other) {
		this->init((int)other.m_capacity);
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
		this->init((int)other.m_capacity);
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

	bool append(char data) {
		return append(&data, 1);
	};

	bool append(const char* data, size_t size) {
		return this->append(data, (int)size);
	}

	bool append(const char* data, int size) {
		if (m_writePos + size > m_capacity)
		{
			double newOpacity_ = m_capacity * 1.5;
			while (newOpacity_ < m_writePos + size)
			{
				newOpacity_ = newOpacity_ * 1.5;
			}

			rsize_t newOpacity = (rsize_t)newOpacity_;
			char* newBuffer = new char[newOpacity];
			memset(newBuffer, 0, newOpacity);
			memcpy_s(newBuffer, newOpacity, m_buffer, m_capacity);
			delete[] m_buffer;
			m_buffer = newBuffer;
			m_capacity = (std::streamsize)newOpacity;
		}

		int rtn = memcpy_s(m_buffer + m_writePos, m_capacity - m_writePos, data, size);
		m_writePos += size;

		return true;
	};

	char* data() {
		return m_buffer + m_readPos;
	};

	char* take(int length_) {
		if (m_readPos == m_writePos) {
			return nullptr;
		}

		char* readPos = m_buffer + m_readPos;
		
		std::streamsize length = length_;

		if (length + m_readPos > m_writePos){
			length = m_writePos - m_readPos;
		}
		char* d = new char[length];
		memcpy_s(d, length, readPos, length);
		m_readPos += length;
		return d;
	}

	const char* FindFirstCrlf() const {
		const char* crlf = std::search(Peek(), BeginWrite(), kCRLF, kCRLF + 2);
		return crlf == BeginWrite() ? nullptr : crlf;
	}

	const char* FindLastCrlf() const {
		const char* crlf = std::find_end(Peek(), BeginWrite(), kCRLF, kCRLF + 2);
		return crlf == BeginWrite() ? nullptr : crlf;
	}

	const char* FindLastCrlfCrlf() const {
		char crlfCrlf[] = "\r\n\r\n";
		const char* crlf = std::find_end(Peek(), BeginWrite(), crlfCrlf, crlfCrlf + 4);
		return crlf == BeginWrite() ? nullptr : crlf;
	}

	void Retrieve(size_t len) {
		if (len <= (size_t)(m_writePos - m_readPos)) {
			m_readPos += len;
			if (m_readPos == m_writePos) {
				m_writePos = 0;
				m_readPos = 0;
			}
		}
		else {
			m_writePos = 0;
			m_readPos = 0;
		}
	}

	void RetrieveUntil(const char* end)
	{
		Retrieve(end - Peek());
	}

	void clear() {
		memset(m_buffer, 0 ,m_capacity);
		m_writePos = 0;
		m_readPos = 0;
	}

	int length() {
		return (int)(m_writePos - m_readPos);
	};

	int capacity() {
		return (int)m_capacity;
	}

	bool empty() {
		if (m_ifs && m_ifs->eof() == false){
			readFileContent();
		}
		else if (m_ifs) {
			m_ifs->close();
			m_ifs = nullptr;
		}

		return m_writePos == 0 ? true : false;
	};

	void setIfstream(std::ifstream* ifs) { m_ifs = ifs; };

	void readFileContent() {
		//memset(m_buffer, 0, m_capacity);
		//m_readPos = 0;
		if (m_readPos != m_writePos)
		{
			return;
		}
		m_readPos = 0;
		m_ifs->read(m_buffer, m_capacity);
		m_writePos = m_ifs->gcount();
	};


	std::ifstream* m_ifs = nullptr;

	char* m_buffer = nullptr;
	std::streamsize m_capacity = 0;
	std::streamsize m_writePos = 0;
	std::streamsize m_readPos = 0;


	////////////////////// copyright
		char* Peek()
		{
			return Begin() + m_readPos;
		}

		const char* Peek() const
		{
			return Begin() + m_readPos;
		}

		char* Begin()
		{
			return m_buffer;
		}

		const char* Begin() const
		{
			return m_buffer;
		}

		char* beginWrite()
		{
			return Begin() + m_writePos;
		}

		const char* BeginWrite() const
		{
			return Begin() + m_writePos;
		}
		static const char kCRLF[];
};
