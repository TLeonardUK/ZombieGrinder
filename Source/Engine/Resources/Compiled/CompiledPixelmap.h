// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_COMPILED_PIXELMAP_
#define _ENGINE_RESOURCES_COMPILED_PIXELMAP_

#include "Engine/Renderer/Textures/PixelmapFormat.h"

struct CompiledPixelmap
{
	u32					 width;
	u32					 height;
	PixelmapFormat::Type format;
	void*				 data;
};

#endif