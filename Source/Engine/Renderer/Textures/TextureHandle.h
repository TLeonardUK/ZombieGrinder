// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTUREHANDLE_
#define _ENGINE_RENDERER_TEXTURES_TEXTUREHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Resources/Reloadable.h"

#include "Engine/Renderer/Textures/Texture.h"

#include <string>

// The texture handle wraps a Texture class instance, and automatically
// reloads the texture if the source file is changed.

class TextureFactory;
class Texture;

class TextureHandle : public Reloadable
{
	MEMORY_ALLOCATOR(TextureHandle, "Rendering");

private:

	Texture*			m_texture;
	std::string			m_url;
	TextureFlags::Type	m_flags;

protected:
	
	friend class TextureFactory;

	// Only texture factory should be able to modify these!
	TextureHandle(const char* url, TextureFlags::Type flags, Texture* texture);
	~TextureHandle();
	
	void Reload();

public:

	// Get/Set
	Texture* Get();
	std::string Get_URL();

};

#endif

