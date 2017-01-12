// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PixelmapFORMAT_
#define _ENGINE_RENDERER_TEXTURES_PixelmapFORMAT_

struct PixelmapFormat
{
	// If you modify this, make sure to update the BPPForPixelmapFormat array
	// in the cpp file.
	enum Type
	{
		// Raw formats.
		R8 = 0,
		R8G8B8A8,
		R8G8B8,

		// Compressed formats, these cannot be read/written to, they are
		// used simply as storage for later use by the graphics driver.
		DXT3,
		DXT5
	};
};

#endif


