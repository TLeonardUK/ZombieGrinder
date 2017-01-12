// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/IO/MacOS/MacOS_FileStream.h"

MacOS_FileStream::MacOS_FileStream(FILE* handle) 
	: m_handle(handle)
	, m_open(true)
{
}

MacOS_FileStream::~MacOS_FileStream()
{
	if (m_open == true)
	{
		Close();
	}
}

bool MacOS_FileStream::IsEOF()
{
	DBG_ASSERT(m_open == true);

	return Position() >= Length();
}

unsigned int MacOS_FileStream::Position()
{
	DBG_ASSERT(m_open == true);

	return (unsigned int)ftell(m_handle);
}

void MacOS_FileStream::Seek(unsigned int offset)
{
	DBG_ASSERT(m_open == true);

	fseek(m_handle, offset, SEEK_SET);
}

unsigned int MacOS_FileStream::Length()
{
	DBG_ASSERT(m_open == true);

	unsigned int position = ftell(m_handle);
	fseek(m_handle, 0, SEEK_END);
	unsigned int remaining = ftell(m_handle);// - position;
	fseek(m_handle, position, SEEK_SET);
	return remaining;
}

void MacOS_FileStream::WriteBuffer(const char* buffer, int offset, int length)
{
	DBG_ASSERT(m_open == true);

	fwrite(buffer + offset, sizeof(char), length, m_handle);
}

void MacOS_FileStream::ReadBuffer(char* buffer, int offset, int length)
{
	DBG_ASSERT(m_open == true);

	fread(buffer + offset, sizeof(char), length, m_handle);
}

void MacOS_FileStream::Close()
{
	DBG_ASSERT(m_open == true);

	fclose(m_handle);
	m_open = false;
}

void MacOS_FileStream::Flush()
{
	fflush(m_handle);
}

#endif