// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PNG_PixelmapFACTORY_
#define _ENGINE_RENDERER_TEXTURES_PNG_PixelmapFACTORY_

#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "libpng/png.h"

class PNGPixelmapFactory : public PixelmapFactory
{
	MEMORY_ALLOCATOR(PNGPixelmapFactory, "Rendering");

private:
	static void libpng_read_function(png_structp pngPtr, png_bytep data, png_size_t length);
	static void libpng_write_function(png_structp pngPtr, png_bytep data, png_size_t length);

public:
	Pixelmap*  Try_Load(const char* url);
	Pixelmap*  Try_Load(Stream* url);
	bool	   Try_Save(const char* url, Pixelmap* texture);
	bool	   Try_Save(Stream* url, Pixelmap* texture);

};

#endif

