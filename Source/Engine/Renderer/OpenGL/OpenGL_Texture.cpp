// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Renderer/OpenGL/OpenGL_Texture.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

#include "libsquish/squish.h"

OpenGL_Texture::OpenGL_Texture(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags)
	: Texture(width, height, pitch, format)
	, m_texture_created(false)
	, m_flags(flags)
	, m_texture_id(0)
{

}

OpenGL_Texture::OpenGL_Texture(Pixelmap* Pixelmap, TextureFlags::Type flags)
	: Texture(Pixelmap)
	, m_texture_created(false)
	, m_flags(flags)
	, m_texture_id(0)
{

}

GLuint OpenGL_Texture::Get_ID() const
{ 
	return m_texture_id; 
}

OpenGL_Texture::~OpenGL_Texture()
{
	if ((m_flags & TextureFlags::PersistSourceData) != 0)
	{
		m_Pixelmap = NULL;
	}
	if (m_texture_created)
	{
		glDeleteTextures(1, &m_texture_id);
		CHECK_GL_ERROR();
	}
}

void OpenGL_Texture::Update()
{
	if (!m_texture_created)
	{
		Upload_Data();
		m_texture_created = true;
	}
}

void OpenGL_Texture::Upload_Data()
{
	// Generate and bind texture.
	GLint original_bind = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &original_bind);
	CHECK_GL_ERROR();
	glActiveTexture(GL_TEXTURE0);
	CHECK_GL_ERROR();

	// Generate and bind texture.
	glGenTextures(1, &m_texture_id);
	CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	CHECK_GL_ERROR();
	
	// And some nice filtering and clamp the texture.
	if ((m_flags & TextureFlags::AllowRepeat) != 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		CHECK_GL_ERROR();
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
		CHECK_GL_ERROR();
	}
//	if ((m_flags & TextureFlags::LinearFilter) != 0)
//	{
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
//	}
//	else
//	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
		CHECK_GL_ERROR();
//	}
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); 
	CHECK_GL_ERROR();

	TextureFormat::Type format	= m_Pixelmap == NULL ? m_format : m_Pixelmap->Get_Texture_Format();
	int width					= m_Pixelmap == NULL ? m_width : m_Pixelmap->Get_Width();
	int height					= m_Pixelmap == NULL ? m_height : m_Pixelmap->Get_Height();
	unsigned char* data			= m_Pixelmap == NULL ? NULL : m_Pixelmap->Get_Data();

	// Upload data to GPU.
	Upload_To_GPU(width, height, format, data);
	
	// Reset binding to previous texture.
	glBindTexture(GL_TEXTURE_2D, original_bind);
	CHECK_GL_ERROR();

	// We don't care about pixelmap data anymore.
	if ((m_flags & TextureFlags::PersistSourceData) == 0)
	{
		if (m_Pixelmap != NULL)
		{
			m_Pixelmap->Free_Data();
		}
	}
}

void OpenGL_Texture::Set_Pixelmap(Pixelmap* Pixelmap)
{
	// Generate and bind texture.
	GLint original_bind = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &original_bind);
	CHECK_GL_ERROR();
	glActiveTexture(GL_TEXTURE0);
	CHECK_GL_ERROR();
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	CHECK_GL_ERROR();
	
	m_format			= Pixelmap->Get_Texture_Format();
	m_width				= Pixelmap->Get_Width();
	m_height			= Pixelmap->Get_Height();
	unsigned char* data	= Pixelmap->Get_Data();

	// Upload data to GPU.
	Upload_To_GPU(m_width, m_height, m_format, data);
	
	glBindTexture(GL_TEXTURE_2D, original_bind);
	CHECK_GL_ERROR();

	// Replace current buffer with new one.
	if (Pixelmap != m_Pixelmap)
	{
		SAFE_DELETE(m_Pixelmap);
		m_Pixelmap = Pixelmap;
	}
}

void OpenGL_Texture::Upload_To_GPU(int width, int height, TextureFormat::Type format, void* data)
{
	switch (format)
	{
	case TextureFormat::R8G8B8:
		{
			if (data != NULL)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			CHECK_GL_ERROR();
			break;
		}
#ifndef PLATFORM_MACOS
	case TextureFormat::R32FG32FB32FA32F:
		{
			if (data != NULL)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
			CHECK_GL_ERROR();
			break;
		}
#endif
	case TextureFormat::R8G8B8A8:
		{
			if (data != NULL)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			CHECK_GL_ERROR();
			break;
		}
	case TextureFormat::DepthFormat:
		{
			if (data != NULL)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);	
			CHECK_GL_ERROR();		
			break;
		}
	case TextureFormat::StencilFormat:
		{
			if (data != NULL)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, width, height, 0, GL_STENCIL_INDEX, GL_FLOAT, data);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, width, height, 0, GL_STENCIL_INDEX, GL_FLOAT, 0);
			CHECK_GL_ERROR();
			break;
		}
	case TextureFormat::Luminosity:
		{
			if (data != NULL)
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
			CHECK_GL_ERROR();
			break;
		}
	/*
	case TextureFormat::DXT3:
		{
			int data_size = squish::GetStorageRequirements(width, height, squish::kDxt3);
			if (data != NULL)
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, width, height, 0, data_size, data);
			else
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, width, height, 0, data_size, 0);
			CHECK_GL_ERROR();
			break;
		}
	case TextureFormat::DXT5:
		{
			int data_size = squish::GetStorageRequirements(width, height, squish::kDxt5);
			if (data != NULL)
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, width, height, 0, data_size, data);
			else
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, width, height, 0, data_size, 0);
			CHECK_GL_ERROR();
			break;
		}
	*/
	default:
		{
			// Format not supported.
			DBG_ASSERT(false);
			break;
		}
	}
}

#endif