// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/IO/BinaryStream.h"

#define BINARY_STREAM_DEFAULT_BUFFER_SIZE 256

BinaryStream::BinaryStream()
	: m_capacity(BINARY_STREAM_DEFAULT_BUFFER_SIZE)
	, m_length(0)
	, m_position(0)
	, m_owns_data(true)
{
	m_data = (char*)Get_Allocator()->Alloc(BINARY_STREAM_DEFAULT_BUFFER_SIZE);
}

BinaryStream::BinaryStream(const char* data, unsigned int data_size)
	: m_capacity(data_size)
	, m_length(data_size)
	, m_position(0)
	, m_owns_data(true)
{
	m_data = (char*)Get_Allocator()->Alloc(data_size);
	memcpy(m_data, data, data_size);
}

BinaryStream::~BinaryStream()
{
	if (m_owns_data == true)
		Get_Allocator()->Free(m_data);
}

char* BinaryStream::Data()
{
	return m_data;
}

char* BinaryStream::Take_Data()
{
	m_owns_data = false;
	return m_data;
}

void BinaryStream::Reserve(unsigned int size)
{
	int capacity = m_capacity;
	while (m_position + size > (unsigned int)capacity)
	{
		capacity *= 2;
	}

	if (capacity == m_capacity)
	{
		return;
	}

	char* new_data = (char*)Get_Allocator()->Alloc(capacity);
	memcpy(new_data, m_data, m_capacity);

	Get_Allocator()->Free(m_data);
	m_data = new_data;
	m_capacity = capacity;
}

void BinaryStream::Reserve_Exactly(unsigned int size)
{
	if (size == m_capacity)
	{
		return;
	}
	
	char* new_data = (char*)Get_Allocator()->Alloc(size);
	memcpy(new_data, m_data, Min(m_capacity, size));

	Get_Allocator()->Free(m_data);
	m_data = new_data;
	m_capacity = size;
}

bool BinaryStream::IsEOF()
{
	return m_position >= m_length;
}

unsigned int BinaryStream::Position()
{
	return m_position;
}

void BinaryStream::Seek(unsigned int offset)
{
	DBG_ASSERT(offset >= 0 && offset <= m_length);
	m_position = offset;
}

unsigned int BinaryStream::Length()
{
	return m_length;
}

void BinaryStream::WriteBuffer(const char* buffer, int offset, int length)
{
	Reserve(length);
	memcpy(m_data + m_position, buffer + offset, length);

	m_position += length;
	if (m_position > m_length)
	{
		m_length = m_position;
	}
}

void BinaryStream::ReadBuffer(char* buffer, int offset, int length)
{
	DBG_ASSERT(m_position + length <= m_length);
	memcpy(buffer + offset, m_data + m_position, length);
	m_position += length;
}

void BinaryStream::Close()
{
	// Nothing to do.
}

void BinaryStream::Flush()
{
	// Nothing to do.
}
