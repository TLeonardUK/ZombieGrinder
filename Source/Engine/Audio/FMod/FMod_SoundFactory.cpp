// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Audio/FMod/FMod_AudioRenderer.h"
#include "Engine/Audio/FMod/FMod_SoundFactory.h"
#include "Engine/Audio/FMod/FMod_Sound.h"

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include "Generic/Helper/StringHelper.h"

Sound* FMod_SoundFactory::Try_Load(CompiledAudioSound* header)
{
	FMod_AudioRenderer* renderer = dynamic_cast<FMod_AudioRenderer*>(AudioRenderer::Get());
	DBG_ASSERT(renderer != NULL);

	FMOD_SYSTEM* system = renderer->Get_FMod_System();
	FMOD_MODE mode = FMOD_CREATESAMPLE | FMOD_3D | FMOD_3D_LINEARSQUAREROLLOFF | FMOD_3D_WORLDRELATIVE  | FMOD_OPENMEMORY;// | FMOD_LOWMEM  | FMOD_CREATECOMPRESSEDSAMPLE;

	FMOD_CREATESOUNDEXINFO exinfo;
	memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	exinfo.length = header->data_length;

	const char* data_to_pass = (char*)header->data;
	FMOD_CREATESOUNDEXINFO* exinfo_to_pass = &exinfo;

	if ((header->flags & SoundFlags::Loop) != 0)
	{
		mode |= FMOD_LOOP_NORMAL;
	}

	if ((header->flags & SoundFlags::Streaming) != 0)
	{
		mode &= ~FMOD_CREATESAMPLE;

	//	mode |= FMOD_NONBLOCKING;
		mode |= FMOD_CREATESTREAM;
		mode |= FMOD_OPENMEMORY;
	}

	FMOD_SOUND* sound = NULL;
	FMOD_RESULT result = FMOD_OK;

	if ((header->flags & SoundFlags::Streaming) != 0)
	{
		result = FMOD_System_CreateStream(system, data_to_pass, mode, exinfo_to_pass, &sound);
	}
	else
	{
		result = FMOD_System_CreateSound(system, data_to_pass, mode, exinfo_to_pass, &sound);
	}

	if (result != FMOD_OK)
	{
		DBG_LOG("FMod failed to load sound from '%s' due to error 0x%08x", header->name, result);
		return NULL;
	}
	else
	{
		DBG_LOG("FMod loaded '%s'", header->name);
	}

	return new FMod_Sound(renderer, sound, header);
}

#endif