// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_ATLASES_ATLASFACTORY_
#define _ENGINE_RENDERER_ATLASES_ATLASFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Engine/IO/Stream.h"
#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasFactory.h"

class AtlasFactory
{
	MEMORY_ALLOCATOR(AtlasFactory, "Rendering");

private:
	static ThreadSafeHashTable<AtlasHandle*, int>	m_loaded_atlases;

	// Static Class
	AtlasFactory();	
	
	// Internal functions.
	static Atlas* Load_Atlas				(const char* url);

public:
		
	// Static methods.
	static AtlasHandle* Load			   (const char* url);
	static Atlas*		Load_Without_Handle(const char* url);

};

#endif

