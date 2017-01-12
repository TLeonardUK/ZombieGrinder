// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

Texture::Texture(int width, int height, int pitch, TextureFormat::Type format)
	: m_width(width)
	, m_height(height)
	, m_pitch(pitch)
	, m_format(format)
	, m_Pixelmap(NULL)
{
}

Texture::Texture(Pixelmap* Pixelmap)
	: m_width(Pixelmap->Get_Width())
	, m_height(Pixelmap->Get_Height())
	, m_pitch(Pixelmap->Get_Pitch())
	, m_format(Pixelmap->Get_Texture_Format())
	, m_Pixelmap(Pixelmap)
{
}

Texture::~Texture()
{
	SAFE_DELETE(m_Pixelmap);
}

int	Texture::Get_Width() const
{
	return m_width;
}

int	Texture::Get_Height() const
{
	return m_height;
}

int	Texture::Get_Pitch() const
{
	return m_pitch;
}

TextureFormat::Type	Texture::Get_Format() const
{
	return m_format;
}

Pixelmap*	Texture::Get_Pixelmap() const
{
	return m_Pixelmap;
}
