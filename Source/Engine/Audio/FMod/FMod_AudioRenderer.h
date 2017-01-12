// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_AUDIO_FMOD_AUDIORENDERER_
#define _ENGINE_AUDIO_FMOD_AUDIORENDERER_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Matrix4.h"
#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Audio/FMod/FMod_SoundFactory.h"

#include "Generic/ThirdParty/FMod/fmod.hpp"
#include "Generic/ThirdParty/FMod/fmod_errors.h"

#include <vector>

class FMod_Sound;

// FMod rendering fun times!
class FMod_AudioRenderer : public AudioRenderer
{
	MEMORY_ALLOCATOR(FMod_AudioRenderer, "Audio"); 

private:
	friend class AudioRenderer;

	FMOD_SYSTEM* m_system;
	Vector3 m_old_position;

	FMod_SoundFactory m_sound_factory;

	static void* F_CALLBACK fmod_alloc	(unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr);
	static void* F_CALLBACK fmod_realloc(void *ptr, unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr);
	static void  F_CALLBACK fmod_free	(void *ptr, FMOD_MEMORY_TYPE type, const char *sourcestr);

	static FMOD_RESULT F_CALLBACK fmod_open (const char* name, unsigned int* filesize, void** handle, void* userdata);
	static FMOD_RESULT F_CALLBACK fmod_close(void* handle, void* userdata);
	static FMOD_RESULT F_CALLBACK fmod_seek (void* handle, unsigned int pos, void* userdata);
	static FMOD_RESULT F_CALLBACK fmod_read (void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata);

	static FMOD_RESULT F_CALLBACK fmod_streaming_callback(FMOD_SOUND *sound, void *data, unsigned int datalen);

	void Dump_Channels();

public:
	FMod_AudioRenderer();
	~FMod_AudioRenderer();

	// FMOD Specific.
	FMOD_SYSTEM* Get_FMod_System();
	
	// Create stream buffer.
	void Set_Listener_Position(int index, Vector3 position);
	void Set_Listener_Count(int count);
	Sound* Create_Streaming_Sound(int sample_rate, int channels, SoundFormat::Type type, AudioRequestPCMCallback callback, void* meta_data);

	// Base functions.	
	void Tick(const FrameTime& time);

};

#endif

#endif