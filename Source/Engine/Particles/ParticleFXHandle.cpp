// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Particles/ParticleFXFactory.h"
#include "Engine/Particles/ParticleFXHandle.h"
#include "Engine/Particles/ParticleFX.h"

ParticleFXHandle::ParticleFXHandle(const char* url, ParticleFX* layout)
	: m_url(url)
	, m_fx(layout)
{
	Add_Reload_Trigger_File(url);
}

ParticleFXHandle::~ParticleFXHandle()
{
	SAFE_DELETE(m_fx);
}
	
void ParticleFXHandle::Reload()
{
	ParticleFX* new_layout = ParticleFXFactory::Load_Without_Handle(m_url.c_str());
	if (new_layout != NULL)
	{
		SAFE_DELETE(m_fx);			
		m_fx = new_layout;

		DBG_LOG("Reloaded changed particlefx: %s", m_url.c_str());
	}
}

ParticleFX* ParticleFXHandle::Get()
{
	return m_fx;
}

std::string ParticleFXHandle::Get_URL()
{
	return m_url;
}