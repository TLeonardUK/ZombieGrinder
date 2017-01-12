// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_ATLASES_ATLASHANDLE_
#define _ENGINE_RENDERER_ATLASES_ATLASHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Atlases/Atlas.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

// The handle wraps a Atlas class instance, and automatically
// reloads the atla if the source file is changed.

class AtlasFactory;
class Atlas;

class AtlasHandle : public Reloadable
{
	MEMORY_ALLOCATOR(AtlasHandle, "Rendering");

private:

	Atlas*				m_atlas;
	std::string			m_url;

protected:
	
	friend class AtlasFactory;

	// Only font factory should be able to modify these!
	AtlasHandle(const char* url, Atlas* atlas);
	~AtlasHandle();

	void Reload();

public:

	// Get/Set
	Atlas* Get();
	std::string Get_URL();

};

#endif

