// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_VIDEOFACTORY_
#define _ENGINE_VIDEOFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Engine/IO/Stream.h"

#include "Engine/Video/Video.h"
#include "Engine/Video/VideoHandle.h"

class VideoFactory
{
	MEMORY_ALLOCATOR(VideoFactory, "Localise");

private:
	static ThreadSafeHashTable<VideoHandle*, int> m_loaded_banks;

protected:
	static Video*		Try_Load(const char* url);

public:

	static std::vector<VideoHandle*> Get_Languages();

	// Dispose.
	static void Dispose();

	// Static methods.
	static VideoHandle* Load(const char* url);
	static Video*	Load_Without_Handle(const char* url);

};

#endif

