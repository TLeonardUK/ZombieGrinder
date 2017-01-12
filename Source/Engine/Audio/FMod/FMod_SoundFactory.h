// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_AUDIO_FMOD_FMODSOUNDFACTORY_
#define _ENGINE_AUDIO_FMOD_FMODSOUNDFACTORY_

#include "Engine/Audio/Sounds/SoundFactory.h"

class FMod_SoundFactory : public SoundFactory
{
	MEMORY_ALLOCATOR(FMod_SoundFactory, "Audio");

private:

public:
	Sound* Try_Load(CompiledAudioSound* header);

};

#endif

#endif