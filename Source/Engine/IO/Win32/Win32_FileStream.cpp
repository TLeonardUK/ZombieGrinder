// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/IO/Win32/Win32_FileStream.h"

Win32_FileStream::Win32_FileStream(FILE* handle) 
	: m_handle(handle)
	, m_open(true)
{
	// Precalculate length. Assumes only read or write, not both at the same time!
	unsigned int position = ftell(m_handle);
	fseek(m_handle, 0, SEEK_END);
	m_length = ftell(m_handle);// - position;
	fseek(m_handle, position, SEEK_SET);
}

Win32_FileStream::~Win32_FileStream()
{
	if (m_open == true)
	{
		Close();
	}
}

bool Win32_FileStream::IsEOF()
{
	DBG_ASSERT(m_open == true);

	return Position() >= Length();
}

unsigned int Win32_FileStream::Position()
{
	DBG_ASSERT(m_open == true);

	return (unsigned int)ftell(m_handle);
}

void Win32_FileStream::Seek(unsigned int offset)
{
	DBG_ASSERT(m_open == true);

	fseek(m_handle, offset, SEEK_SET);
}

unsigned int Win32_FileStream::Length()
{
	DBG_ASSERT(m_open == true);
	return m_length;
}

void Win32_FileStream::WriteBuffer(const char* buffer, int offset, int length)
{
	DBG_ASSERT(m_open == true);

	fwrite(buffer + offset, sizeof(char), length, m_handle);
}

void Win32_FileStream::ReadBuffer(char* buffer, int offset, int length)
{
	DBG_ASSERT(m_open == true);

	fread(buffer + offset, sizeof(char), length, m_handle);
}

void Win32_FileStream::Close()
{
	DBG_ASSERT(m_open == true);

	fclose(m_handle);
	m_open = false;
}

void Win32_FileStream::Flush()
{
	fflush(m_handle);
}

#endif