// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Atlases/AtlasFactory.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/Atlas.h"

AtlasHandle::AtlasHandle(const char* url, Atlas* atlas)
	: m_url(url)
	, m_atlas(atlas)
{
	Add_Reload_Trigger_File(url);
}

AtlasHandle::~AtlasHandle()
{
	SAFE_DELETE(m_atlas);
}
	
void AtlasHandle::Reload()
{
	Atlas* new_atlas = AtlasFactory::Load_Without_Handle(m_url.c_str());
	if (new_atlas != NULL)
	{
		SAFE_DELETE(m_atlas);			
		m_atlas = new_atlas;

		DBG_LOG("Reloaded changed atlas: %s", m_url.c_str());
	}
}

Atlas* AtlasHandle::Get()
{
	return m_atlas;
}

std::string AtlasHandle::Get_URL()
{
	return m_url;
}
