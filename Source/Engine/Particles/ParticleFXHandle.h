// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_PARTICLES_PARTICLEFXHANDLE_
#define _ENGINE_UI_PARTICLES_PARTICLEFXHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

class ParticleFXFactory;
class ParticleFX;

class ParticleFXHandle : public Reloadable
{
	MEMORY_ALLOCATOR(ParticleFXHandle, "UI");

private:

	ParticleFX*			m_fx;
	std::string			m_url;

protected:
	
	friend class ParticleFXFactory;

	ParticleFXHandle(const char* url, ParticleFX* layout);
	~ParticleFXHandle();
	
	void Reload();

public:

	// Get/Set
	ParticleFX* Get();
	std::string Get_URL();

};

#endif

