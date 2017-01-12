// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_STREAM_WIN32_FILESTREAM_
#define _ENGINE_STREAM_WIN32_FILESTREAM_

#include "Engine/IO/Stream.h"

class Win32_FileStream : public Stream
{
	MEMORY_ALLOCATOR(Win32_FileStream, "IO");

private:
	FILE*			m_handle;
	bool			m_open;
	unsigned int	m_length;

public:

	Win32_FileStream(FILE* handle);
	~Win32_FileStream();

	// Members that have to be overidden.
	bool			IsEOF		();
	unsigned int	Position	();
	void			Seek		(unsigned int offset);
	unsigned int	Length		();
	void			WriteBuffer	(const char* buffer, int offset, int length);
	void			ReadBuffer	(char* buffer, int offset, int length);
	void			Close		();
	void			Flush		();

};

#endif

#endif