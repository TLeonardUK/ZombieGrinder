// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _ENGINE_STREAM_LINUX_FILESTREAM_
#define _ENGINE_STREAM_LINUX_FILESTREAM_

#include "Engine/IO/Stream.h"

#include <stdio.h>

class Linux_FileStream : public Stream
{
	MEMORY_ALLOCATOR(Linux_FileStream, "IO");

private:
	bool m_open;
	FILE* m_handle;

public:

	Linux_FileStream(FILE* handle);
	~Linux_FileStream();

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