// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_SOUNDS_SOUNDHANDLE_
#define _ENGINE_AUDIO_SOUNDS_SOUNDHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Audio/Sounds/Sound.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

// The sound handle wraps a Sound class instance, and automatically
// reloads the sound if the source file is changed.

class SoundFactory;
class Sound;

class SoundHandle : public Reloadable
{
	MEMORY_ALLOCATOR(SoundHandle, "Audio");

private:

	Sound*				m_sound;
	std::string			m_url;

protected:
	
	friend class SoundFactory;

	// Only texture factory should be able to modify these!
	SoundHandle(const char* url, Sound* sound);
	~SoundHandle();
	
	void Reload();

public:

	// Get/Set
	const Sound* Get();
	std::string Get_URL();

};

#endif

