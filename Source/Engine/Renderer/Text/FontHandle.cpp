// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/Font.h"

FontHandle::FontHandle(const char* url,  FontFlags::Type flags, Font* font)
	: m_url(url)
	, m_font(font)
	, m_flags(flags)
{
	Add_Reload_Trigger_File(url);
}

FontHandle::~FontHandle()
{
	SAFE_DELETE(m_font);
}
	
void FontHandle::Reload()
{
	Font* new_font = FontFactory::Load_Without_Handle(m_url.c_str(), m_flags);
	if (new_font != NULL)
	{
		SAFE_DELETE(m_font);			
		m_font = new_font;

		DBG_LOG("Reloaded changed font: %s", m_url.c_str());
	}
}

Font* FontHandle::Get()
{
	return m_font;
}

std::string FontHandle::Get_URL()
{
	return m_url;
}
