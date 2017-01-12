// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_NULL_TEXTURE_
#define _ENGINE_RENDERER_NULL_TEXTURE_

#include "Engine/Renderer/Textures/Texture.h"

#include "Engine/Renderer/Null/Null_Renderer.h"

class Null_Texture : public Texture
{
	MEMORY_ALLOCATOR(Null_Texture, "Rendering");

private:
	friend class Null_Renderer;

	TextureFlags::Type m_flags;

private:

	// Constructor!
	Null_Texture(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags);
	Null_Texture(Pixelmap* Pixelmap, TextureFlags::Type flags);

public:

	// Destructor!
	~Null_Texture();

	// Sets the Pixelmap of this texture!
	void Set_Pixelmap(Pixelmap* Pixelmap);

};

#endif

