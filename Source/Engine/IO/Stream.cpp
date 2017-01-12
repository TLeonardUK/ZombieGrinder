// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/IO/Stream.h"

Stream::Stream()
	: m_buffer(NULL)
	, m_buffer_size(0)
{
}

Stream::~Stream()
{
	SAFE_DELETE_ARRAY(m_buffer);
}

unsigned int Stream::Bytes_Remaining()
{
	return Length() - Position();
}

void Stream::WriteLine(const char* str)
{
	WriteBuffer(str, 0, strlen(str));
	WriteBuffer("\r\n", 0, 2);
}

void Stream::WriteString(const char* str)
{
	WriteBuffer(str, 0, strlen(str));
}

void Stream::WriteNullTerminatedString(const char* str)
{
	WriteBuffer(str, 0, strlen(str) + 1);
}

const char* Stream::ReadNullTerminatedString()
{
	int start = Position();
	while (true)
	{
	//	DBG_ASSERT(Bytes_Remaining() > 0);

		char chr = Read<char>();
		if (chr == 0)
			break;
	}
	int end = Position();
	Seek(start);
	return ReadString(end - start);
}

const char*	Stream::ReadLine()
{
	bool exit   = false;
	int  offset = 0;

	while (IsEOF() == false && exit == false)
	{
		char chr   = Read<char>();
		bool store = true;

		if (chr == '\r')
		{
			if (IsEOF() == false)
			{
				char chr2 = Read<char>();
				if (chr2 == '\n')
				{
					store = false;
					exit = true;
				}
				else
				{
					Seek(Position() - 1);
				}
			}
		}
		else if (chr == '\n')
		{
			store = false;
			exit = true;
		}

		if (store == true)
		{
			if (m_buffer == NULL || m_buffer_size <= offset + 1)
			{
				char* old_buffer		= m_buffer;
				int   old_buffer_size	= m_buffer_size;
				m_buffer				= new char[(m_buffer_size + 1) * 2];
				m_buffer_size			= (m_buffer_size + 1) * 2;

				if (old_buffer)
				{
					memcpy(m_buffer, old_buffer, old_buffer_size);
					SAFE_DELETE_ARRAY(old_buffer);
				}
				else
				{
				//	memset(m_buffer, 0, m_buffer_size);
				}
			}

			m_buffer[offset] = chr;
			offset++;
		}
	}

	m_buffer[offset] = '\0';

	return m_buffer;
}

const char*	Stream::ReadString(int length)
{
	if (m_buffer == NULL || m_buffer_size <= length)
	{
		SAFE_DELETE_ARRAY(m_buffer);
		m_buffer = new char[length + 1];
		m_buffer_size = length + 1;
	}

	if (length > 0)
	{
		ReadBuffer(m_buffer, 0, length);
	}

	m_buffer[length] = '\0';
	
	return m_buffer;
}
