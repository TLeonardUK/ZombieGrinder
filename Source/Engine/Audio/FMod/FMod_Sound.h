// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_AUDIO_FMOD_FMODSOUND_
#define _ENGINE_AUDIO_FMOD_FMODSOUND_

#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/AudioRenderer.h"
#include "Generic/ThirdParty/FMod/fmod.hpp"

class FMod_AudioRenderer;
struct CompiledAudioSound;

class FMod_Sound : public Sound
{
	MEMORY_ALLOCATOR(FMod_Sound, "Audio"); 

private:
	friend class FMod_AudioRenderer;

private:
	FMOD_SOUND*				m_sound;
	FMod_AudioRenderer*		m_renderer;

	AudioRequestPCMCallback m_callback;
	void*					m_callback_metadata;

	CompiledAudioSound*		m_resource;
	std::string				m_name;
	int						m_name_hash;

protected:
	void Set_Sound(FMOD_SOUND* sound);

public:

	// Destructor!
	FMod_Sound(FMod_AudioRenderer* renderer, FMOD_SOUND* sound, CompiledAudioSound* resource);
	FMod_Sound(FMod_AudioRenderer* renderer, AudioRequestPCMCallback callback, void* callback_metadata);
	~FMod_Sound();

	// Properties.
	std::string Get_Name() const;
	int Get_Name_Hash() const;
	CompiledAudioSound* Get_Compiled() const;

	// Actual sound stuff!
	void Play(SoundChannel* channel = NULL, bool start_paused = false) const;
	void Play(SoundChannel* channel, Vector3 position, bool start_paused = false) const;
	SoundChannel* Allocate_Channel(bool unstealable = false) const;

};

#endif

#endif