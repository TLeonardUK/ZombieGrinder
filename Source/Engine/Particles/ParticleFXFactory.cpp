// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Particles/ParticleFXFactory.h"
#include "Engine/Particles/ParticleFX.h"
#include "Engine/Particles/ParticleFXHandle.h"

#include "Generic/Helper/StringHelper.h"

ThreadSafeHashTable<ParticleFXHandle*, int> ParticleFXFactory::m_loaded_fx;

ParticleFXFactory::ParticleFXFactory()
{
}

ParticleFXFactory::~ParticleFXFactory()
{
}

ParticleFXHandle* ParticleFXFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_fx.Contains(url_hash))
	{
		DBG_LOG("Loaded particle fx from cache: %s", url);
		return m_loaded_fx.Get(url_hash);
	}

	// Try and load texture!
	ParticleFX* layout = Load_Without_Handle(url);
	if (layout != NULL)
	{		
		ParticleFXHandle* handle = new ParticleFXHandle(url, layout);

		m_loaded_fx.Set(url_hash, handle);
		
		DBG_LOG("Loaded particle fx: %s", url);

		return handle;
	}
	else
	{
		DBG_LOG("Failed to load particle fx: %s", url);
	}

	return NULL;
}

ParticleFX* ParticleFXFactory::Load_Without_Handle(const char* url)
{
	return ParticleFX::Load(url);
}

void ParticleFXFactory::Dispose()
{
	for (ThreadSafeHashTable<ParticleFXHandle*, int>::Iterator iter = m_loaded_fx.Begin(); iter != m_loaded_fx.End(); iter++)
	{
		ParticleFXHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_fx.Clear();
}
