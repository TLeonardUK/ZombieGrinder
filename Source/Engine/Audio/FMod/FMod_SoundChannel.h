// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_AUDIO_FMOD_FMODSOUNDCHANNEL_
#define _ENGINE_AUDIO_FMOD_FMODSOUNDCHANNEL_

#include "Engine/Audio/Sounds/SoundChannel.h"

#include "Engine/Audio/FMod/FMod_AudioRenderer.h"

class FMod_SoundChannel : public SoundChannel
{
	MEMORY_ALLOCATOR(FMod_SoundChannel, "Audio"); 

private:
	friend class FMod_AudioRenderer;

private:
	FMod_AudioRenderer* m_renderer;
	FMOD_CHANNEL*		m_channel;

	float				m_default_frequency;
	int					m_default_priority;

	float				m_pan;

public:
	bool					Is_Unstealable;

public:

	// Destructor!
	FMod_SoundChannel(FMod_AudioRenderer* renderer, FMOD_CHANNEL* channel);
	~FMod_SoundChannel();

	// Sound channel stuff.
	void  Set_Volume(float volume);
	float Get_Volume();

	void  Set_Spatial_Position(Vector3 volume);
	Vector3 Get_Spatial_Position();

	void  Set_Position(float volume);
	float Get_Position();

	void  Set_Pan(float pan);
	float Get_Pan();
	
	void  Set_Rate(float rate);
	float Get_Rate();

	bool Is_Paused();
	bool Is_Playing();
	bool Is_Muted();

	void Pause();
	void Resume();

	void Mute();
	void Unmute();

	// FMOD specific stuff.
	FMOD_CHANNEL* Get_FMod_Channel();
	void Set_FMod_Channel(FMOD_CHANNEL* sound);
	void Set_FMod_Sound(FMOD_SOUND* sound);

};

#endif

#endif