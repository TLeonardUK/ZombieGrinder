// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_NULL_NULLSOUND_
#define _ENGINE_AUDIO_NULL_NULLSOUND_

#include "Engine/Audio/Sounds/Sound.h"

class Null_AudioRenderer;
struct CompiledAudioSound;

class Null_Sound : public Sound
{
	MEMORY_ALLOCATOR(Null_Sound, "Audio");

private:
	friend class Null_AudioRenderer;

private:
	Null_AudioRenderer* m_renderer;
	CompiledAudioSound* m_resource;

public:

	// Destructor!
	Null_Sound(Null_AudioRenderer* renderer, CompiledAudioSound* resource);
	~Null_Sound();

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

