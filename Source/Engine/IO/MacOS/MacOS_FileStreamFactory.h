// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_STREAM_MACOS_FILESTREAMFACTORY_
#define _ENGINE_STREAM_MACOS_FILESTREAMFACTORY_

#include "Engine/IO/StreamFactory.h"

class MacOS_FileStreamFactory : public StreamFactory
{
	MEMORY_ALLOCATOR(MacOS_FileStreamFactory, "IO");

public:
	Stream*	Try_Open				(const char* url, StreamMode::Type mode);
	u64     Try_Get_Last_Modified	(const char* url);

};

#endif

#endif