// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_SOUNDS_SOUND_
#define _ENGINE_AUDIO_SOUNDS_SOUND_

#include "Generic/Types/Vector3.h"

#include <string>

class SoundChannel;
struct CompiledAudioSound;

struct SoundFlags
{
	enum Type
	{
		NONE = 0,
		Loop = 1,
		Streaming = 2
	};
};

struct SoundFormat
{
	enum Type
	{
		// Raw Formats
		PCM16 = 0,

		// File Formats
		OGG = 1,
		IT = 2,
		WAV = 3
	};
};

class Sound
{
	MEMORY_ALLOCATOR(Sound, "Audio");

private:
	
protected:

public:

	// Destructor!
	virtual ~Sound();

	// Properties.
	virtual std::string Get_Name() const = 0;
	virtual int Get_Name_Hash() const = 0;
	virtual CompiledAudioSound* Get_Compiled() const = 0;

	// Actual sound stuff!
	virtual void		  Play				(SoundChannel* channel = NULL, bool start_paused = false) const = 0;
	virtual void		  Play				(SoundChannel* channel, Vector3 position, bool start_paused = false) const = 0;
	virtual SoundChannel* Allocate_Channel	(bool unstealable = false) const = 0;

};

#endif

