// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Textures/PixelmapFactory.h"
#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Renderer.h"

#include "Generic/Helper/StringHelper.h"

ThreadSafeHashTable<TextureHandle*, int>	TextureFactory::m_loaded_textures;

bool TextureFactory::Save(const char* url, Texture* texture, TextureFlags::Type flags)
{
	bool result = PixelmapFactory::Save(url, texture->Get_Pixelmap());
	if (result == true)
	{
		DBG_LOG("Saved texture: %s", url);
	}
	else
	{
		DBG_LOG("Failed to save texture: %s", url);
	}
	return result;
}

TextureHandle* TextureFactory::Load(const char* url, TextureFlags::Type flags)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_textures.Contains(url_hash))
	{
		DBG_LOG("Loaded texture from cache: %s", url);
		return m_loaded_textures.Get(url_hash);
	}

	// Try and load texture!
	Texture* texture = Load_Without_Handle(url, flags);
	if (texture != NULL)
	{		
		TextureHandle* handle = new TextureHandle(url, flags, texture);
		
		m_loaded_textures.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Texture* TextureFactory::Load_Without_Handle(const char* url, TextureFlags::Type flags)
{
	Pixelmap* result = PixelmapFactory::Load(url);
	if (result != NULL)
	{
		// Create texture.
		Texture* texture = Renderer::Get()->Create_Texture(result, flags);
		if (texture == NULL)
		{
			DBG_LOG("Failed to load texture: %s", url);
			delete result;
			return NULL;
		}
		else
		{
			DBG_LOG("Loaded texture: %s", url);
			return texture;
		}
	}
	else
	{
		DBG_LOG("Failed to load texture: %s", url);
	}
	return NULL;
}

void TextureFactory::Dispose()
{
	for (ThreadSafeHashTable<TextureHandle*, int>::Iterator iter = m_loaded_textures.Begin(); iter != m_loaded_textures.End(); iter++)
	{
		TextureHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_textures.Clear();
}
