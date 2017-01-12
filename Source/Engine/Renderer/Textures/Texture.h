// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTURE_
#define _ENGINE_RENDERER_TEXTURES_TEXTURE_

#include "Engine/Renderer/Textures/PixelmapFormat.h"
#include "Engine/Renderer/Textures/TextureFormat.h"

class Pixelmap;

struct TextureFlags
{
	enum Type
	{
		NONE = 0,
		AllowRepeat = 1,
		LinearFilter = 2,
		PersistSourceData = 4
	};
};

class Texture
{
	MEMORY_ALLOCATOR(Texture, "Rendering");

protected:
	Pixelmap*				m_Pixelmap;
	int					m_width;
	int					m_height;
	int					m_pitch;
	TextureFormat::Type m_format;
	
protected:

	// Constructor!
	Texture(int width, int height, int pitch, TextureFormat::Type format);
	Texture(Pixelmap* Pixelmap);

public:

	// Destructor!
	virtual ~Texture();

	// Members that have to be overidden.
	Pixelmap*							Get_Pixelmap	 () const;

	// Note: When setting data ownership of the Pixelmap is transfered to the texture 
	//		 object which will deal with deallocation.
	virtual void					Set_Pixelmap	 (Pixelmap* Pixelmap) = 0;

	int								Get_Width	 () const;
	int								Get_Height	 () const;
	int								Get_Pitch	 () const;
	TextureFormat::Type				Get_Format	 () const;
};

#endif

