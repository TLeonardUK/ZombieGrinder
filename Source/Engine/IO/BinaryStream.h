// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_BINARY_STREAM_
#define _ENGINE_BINARY_STREAM_

#include "Engine/IO/Stream.h"


class BinaryStream : public Stream
{
	MEMORY_ALLOCATOR(BinaryStream, "IO");

private:
	char*			m_data;
	unsigned int	m_capacity;
	unsigned int	m_length;
	unsigned int	m_position;
	bool			m_owns_data;

public:
	BinaryStream();
	BinaryStream(const char* data, unsigned int data_size);
	~BinaryStream();

	char* Data();
	char* Take_Data();

	void Reserve(unsigned int size);
	void Reserve_Exactly(unsigned int size);

	bool IsEOF();
	unsigned int Position();
	void Seek(unsigned int offset);
	unsigned int Length();
	void WriteBuffer(const char* buffer, int offset, int length);
	void ReadBuffer(char* buffer, int offset, int length);
	void Close();
	void Flush();

};

#endif

