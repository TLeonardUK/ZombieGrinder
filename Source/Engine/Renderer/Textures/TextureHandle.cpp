// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Textures/Texture.h"

TextureHandle::TextureHandle(const char* url,  TextureFlags::Type flags, Texture* texture)
	: m_url(url)
	, m_texture(texture)
	, m_flags(flags)
{
	Add_Reload_Trigger_File(url);
}

TextureHandle::~TextureHandle()
{
	SAFE_DELETE(m_texture);
}
	
void TextureHandle::Reload()
{
	Texture* new_texture = TextureFactory::Load_Without_Handle(m_url.c_str(), m_flags);
	if (new_texture != NULL)
	{
		SAFE_DELETE(m_texture);			
		m_texture = new_texture;

		DBG_LOG("Reloaded changed texture: %s", m_url.c_str());
	}
}

Texture* TextureHandle::Get()
{
	return m_texture;
}

std::string TextureHandle::Get_URL()
{
	return m_url;
}