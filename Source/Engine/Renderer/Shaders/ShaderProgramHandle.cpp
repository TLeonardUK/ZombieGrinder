// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgramHandle.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"

ShaderProgramHandle::ShaderProgramHandle(const char* url, ShaderProgram* bank)
	: m_url(url)
	, m_bank(bank)
{
	Add_Reload_Trigger_File(url);
}

ShaderProgramHandle::~ShaderProgramHandle()
{
	SAFE_DELETE(m_bank);
}

void ShaderProgramHandle::Reload()
{
	ShaderProgram* new_sound = ShaderFactory::Load_Without_Handle(m_url.c_str());
	if (new_sound != NULL)
	{
		SAFE_DELETE(m_bank);			
		m_bank = new_sound;

		DBG_LOG("Reloaded changed shader: %s", m_url.c_str());
	}
}

ShaderProgram* ShaderProgramHandle::Get()
{
	return m_bank;
}

std::string ShaderProgramHandle::Get_URL()
{
	return m_url;
}
