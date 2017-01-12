// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Audio/Null/Null_AudioRenderer.h"
#include "Engine/Audio/Null/Null_SoundFactory.h"
#include "Engine/Audio/Null/Null_Sound.h"

Sound* Null_SoundFactory::Try_Load(CompiledAudioSound* header)
{
	Null_AudioRenderer* renderer = dynamic_cast<Null_AudioRenderer*>(AudioRenderer::Get());
	DBG_ASSERT(renderer != NULL);

	return new Null_Sound(renderer, header);
}