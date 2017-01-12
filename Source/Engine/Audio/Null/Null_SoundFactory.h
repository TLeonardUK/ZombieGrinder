// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_NULL_NULLSOUNDFACTORY_
#define _ENGINE_AUDIO_NULL_NULLSOUNDFACTORY_

#include "Engine/Audio/Sounds/SoundFactory.h"

class Null_SoundFactory : public SoundFactory
{
	MEMORY_ALLOCATOR(Null_SoundFactory, "Audio");

private:

public:
	Sound* Try_Load(CompiledAudioSound* header);

};

#endif

