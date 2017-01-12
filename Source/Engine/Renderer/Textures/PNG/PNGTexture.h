// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PNG_TEXTURE_
#define _ENGINE_RENDERER_TEXTURES_PNG_TEXTURE_

#include "Engine/Renderer/Textures/Texture.h"

class PNGTexture : public Texture
{
private:

public:
	PNGTexture();
	~PNGTexture();

	unsigned int			Get_Width	 ();
	unsigned int			Get_Height	 ();
	unsigned int			Get_Pitch	 ();
	const unsigned char*	Get_Data	 ();
	TextureFormat::Type		Get_Format	 ();

};

#endif

