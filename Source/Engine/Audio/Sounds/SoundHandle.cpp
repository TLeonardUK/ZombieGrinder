// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Audio/Sounds/SoundFactory.h"
#include "Engine/Audio/Sounds/SoundHandle.h"
#include "Engine/Audio/Sounds/Sound.h"

SoundHandle::SoundHandle(const char* url, Sound* sound)
	: m_url(url)
	, m_sound(sound)
{
	Add_Reload_Trigger_File(url);
}

SoundHandle::~SoundHandle()
{
	SAFE_DELETE(m_sound);
}
	
void SoundHandle::Reload()
{
	Sound* new_sound = SoundFactory::Load_Without_Handle(m_url.c_str());
	if (new_sound != NULL)
	{
		SAFE_DELETE(m_sound);			
		m_sound = new_sound;

		DBG_LOG("Reloaded changed sound: %s", m_url.c_str());
	}
}

const Sound* SoundHandle::Get()
{
	return m_sound;
}

std::string SoundHandle::Get_URL()
{
	return m_url;
}
