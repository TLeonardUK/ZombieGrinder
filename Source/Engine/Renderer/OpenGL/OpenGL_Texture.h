// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_RENDERER_OPENGL_TEXTURE_
#define _ENGINE_RENDERER_OPENGL_TEXTURE_

#include "Engine/Renderer/Textures/Texture.h"

#include "Engine/Renderer/OpenGL/OpenGL_Renderer.h"

class OpenGL_Texture : public Texture
{
	MEMORY_ALLOCATOR(OpenGL_Texture, "Rendering");

private:
	friend class OpenGL_Renderer;

	GLuint m_texture_id;
	bool m_texture_created;

	TextureFlags::Type m_flags;

private:

	// Constructor!
	OpenGL_Texture(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags);
	OpenGL_Texture(Pixelmap* Pixelmap, TextureFlags::Type flags);

	void Upload_Data();
	void Upload_To_GPU(int width, int height, TextureFormat::Type format, void* data);

public:

	// Destructor!
	~OpenGL_Texture();

	// Binding information.
	GLuint Get_ID() const;
	
	void Set_Pixelmap(Pixelmap* Pixelmap);

	// Uploads data to the gpu if it has changed.
	void Update();


};

#endif

#endif