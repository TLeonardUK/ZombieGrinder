// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_SOUNDS_SOUNDCHANNEL_
#define _ENGINE_AUDIO_SOUNDS_SOUNDCHANNEL_

#include "Generic/Types/Vector3.h"

class SoundChannel
{
	MEMORY_ALLOCATOR(SoundChannel, "Audio");

private:
	
protected:

public:

	// Destructor!
	virtual ~SoundChannel();

	// Channel functionality!
	virtual void  Set_Volume(float volume) = 0;
	virtual float Get_Volume() = 0;

	virtual void  Set_Spatial_Position(Vector3 volume) = 0;
	virtual Vector3 Get_Spatial_Position() = 0;

	virtual void  Set_Position(float volume) = 0;
	virtual float Get_Position() = 0;

	virtual void  Set_Pan(float pan) = 0;
	virtual float Get_Pan() = 0;
	
	virtual void  Set_Rate(float rate) = 0;
	virtual float Get_Rate() = 0;

	virtual bool Is_Paused() = 0;
	virtual bool Is_Playing() = 0;
	virtual bool Is_Muted() = 0;

	virtual void Pause() = 0;
	virtual void Resume() = 0;

	virtual void Mute() = 0;
	virtual void Unmute() = 0;

};

#endif

