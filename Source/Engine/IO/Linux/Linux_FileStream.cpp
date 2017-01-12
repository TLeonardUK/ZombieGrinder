// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Engine/IO/Linux/Linux_FileStream.h"

Linux_FileStream::Linux_FileStream(FILE* handle) 
	: m_handle(handle)
	, m_open(true)
{
}

Linux_FileStream::~Linux_FileStream()
{
	if (m_open == true)
	{
		Close();
	}
}

bool Linux_FileStream::IsEOF()
{
	DBG_ASSERT(m_open == true);

	return Position() >= Length();
}

unsigned int Linux_FileStream::Position()
{
	DBG_ASSERT(m_open == true);

	return (unsigned int)ftell(m_handle);
}

void Linux_FileStream::Seek(unsigned int offset)
{
	DBG_ASSERT(m_open == true);

	fseek(m_handle, offset, SEEK_SET);
}

unsigned int Linux_FileStream::Length()
{
	DBG_ASSERT(m_open == true);

	unsigned int position = ftell(m_handle);
	fseek(m_handle, 0, SEEK_END);
	unsigned int remaining = ftell(m_handle);// - position;
	fseek(m_handle, position, SEEK_SET);
	return remaining;
}

void Linux_FileStream::WriteBuffer(const char* buffer, int offset, int length)
{
	DBG_ASSERT(m_open == true);

	fwrite(buffer + offset, sizeof(char), length, m_handle);
}

void Linux_FileStream::ReadBuffer(char* buffer, int offset, int length)
{
	DBG_ASSERT(m_open == true);

	fread(buffer + offset, sizeof(char), length, m_handle);
}

void Linux_FileStream::Close()
{
	DBG_ASSERT(m_open == true);

	fclose(m_handle);
	m_open = false;
}

void Linux_FileStream::Flush()
{
	fflush(m_handle);
}

#endif