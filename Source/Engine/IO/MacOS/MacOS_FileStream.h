// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_STREAM_MACOS_FILESTREAM_
#define _ENGINE_STREAM_MACOS_FILESTREAM_

#include "Engine/IO/Stream.h"

class MacOS_FileStream : public Stream
{
	MEMORY_ALLOCATOR(MacOS_FileStream, "IO");

private:
	bool m_open;
	FILE* m_handle;

public:

	MacOS_FileStream(FILE* handle);
	~MacOS_FileStream();

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