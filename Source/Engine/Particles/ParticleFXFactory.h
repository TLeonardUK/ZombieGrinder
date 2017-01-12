// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_PARTICLES_PARTICLEFXFACTORY_
#define _ENGINE_UI_PARTICLES_PARTICLEFXFACTORY_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/ThreadSafeHashTable.h"

class ParticleFX;
class ParticleFXHandle;

class ParticleFXFactory
{
	MEMORY_ALLOCATOR(ParticleFXFactory, "UI");

protected:
	static ThreadSafeHashTable<ParticleFXHandle*, int> m_loaded_fx;

	ParticleFXFactory();
	~ParticleFXFactory();

public:

	// Disposal.
	static void Dispose();
		
	// Static methods.
	static ParticleFXHandle* Load				  (const char* url);
	static ParticleFX*		 Load_Without_Handle(const char* url);

};

#endif

