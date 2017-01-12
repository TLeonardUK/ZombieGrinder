// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_NULL_AUDIORENDERER_
#define _ENGINE_AUDIO_NULL_AUDIORENDERER_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Matrix4.h"
#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Audio/Null/Null_SoundFactory.h"

#include <vector>

class Null_Sound;

class Null_AudioRenderer : public AudioRenderer
{
	MEMORY_ALLOCATOR(Null_AudioRenderer, "Audio");

private:
	friend class AudioRenderer;
	
	Null_SoundFactory m_sound_factory;

	Null_AudioRenderer();
	~Null_AudioRenderer();

public:
	
	// Base functions.	
	void Tick(const FrameTime& time);
		
	// Create stream buffer.
	void Set_Listener_Count(int count);
	void Set_Listener_Position(int index, Vector3 position);
	Sound* Create_Streaming_Sound(int sample_rate, int channels, SoundFormat::Type type, AudioRequestPCMCallback callback, void* meta_data);

};

#endif

