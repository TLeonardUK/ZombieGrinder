// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_NULL_NULLSOUNDCHANNEL_
#define _ENGINE_AUDIO_NULL_NULLSOUNDCHANNEL_

#include "Engine/Audio/Sounds/SoundChannel.h"

#include "Engine/Audio/Null/Null_AudioRenderer.h"

class Null_SoundChannel : public SoundChannel
{
	MEMORY_ALLOCATOR(Null_SoundChannel, "Audio");

private:
	friend class Null_AudioRenderer;

private:
	Null_AudioRenderer* m_renderer;
	
	float m_volume;
	float m_position;
	float m_pan;
	float m_rate;
	bool m_paused;
	bool m_muted;

public:

	// Destructor!
	Null_SoundChannel(Null_AudioRenderer* renderer);
	~Null_SoundChannel();

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

};

#endif

