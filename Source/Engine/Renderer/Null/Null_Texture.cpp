// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Null/Null_Texture.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

Null_Texture::Null_Texture(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags)
	: Texture(width, height, pitch, format)
	, m_flags(flags)
{

}

Null_Texture::Null_Texture(Pixelmap* Pixelmap, TextureFlags::Type flags)
	: Texture(Pixelmap)
	, m_flags(flags)
{

}

Null_Texture::~Null_Texture()
{
	if ((m_flags & TextureFlags::PersistSourceData) != 0)
	{
		m_Pixelmap = NULL;
	}
}

void Null_Texture::Set_Pixelmap(Pixelmap* Pixelmap)
{
	// Replace current buffer with new one.
	if (Pixelmap != m_Pixelmap && (m_flags & TextureFlags::PersistSourceData) == 0)
	{
		SAFE_DELETE(m_Pixelmap);
		m_Pixelmap = Pixelmap;
	}
}

