// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_COMPILED_AUDIO_
#define _ENGINE_RESOURCES_COMPILED_AUDIO_

#include "Engine/Audio/Sounds/Sound.h"

struct CompiledAudioSound
{
	char*					name;
	u32						name_hash;
	SoundFlags::Type		flags;
	SoundFormat::Type		format;
	void*					data;
	u32						data_length;
};

#endif