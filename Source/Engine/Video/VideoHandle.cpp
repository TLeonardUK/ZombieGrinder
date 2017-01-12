// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Video/VideoFactory.h"
#include "Engine/Video/VideoHandle.h"
#include "Engine/Video/Video.h"

VideoHandle::VideoHandle(const char* url, Video* bank)
	: m_url(url)
	, m_bank(bank)
{
	Add_Reload_Trigger_File(url);
}

VideoHandle::~VideoHandle()
{
	SAFE_DELETE(m_bank);
}

void VideoHandle::Reload()
{
	Video* new_sound = VideoFactory::Load_Without_Handle(m_url.c_str());
	if (new_sound != NULL)
	{
		SAFE_DELETE(m_bank);
		m_bank = new_sound;

		DBG_LOG("Reloaded changed video: %s", m_url.c_str());
	}
}

Video* VideoHandle::Get()
{
	return m_bank;
}

std::string VideoHandle::Get_URL()
{
	return m_url;
}
