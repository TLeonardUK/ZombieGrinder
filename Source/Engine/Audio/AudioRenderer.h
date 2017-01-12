// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_AUDIORENDERER_
#define _ENGINE_AUDIO_AUDIORENDERER_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Matrix4.h"
#include "Generic/Types/AABB.h"
#include "Generic/Types/Frustum.h"
#include "Generic/Types/Color.h"
#include "Generic/Types/Rect2D.h"

#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/RenderTarget.h"
#include "Engine/Renderer/Material.h"

#include "Engine/Audio/Sounds/SoundChannel.h"
#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Demo/DemoManager.h"
#include "Engine/IO/BinaryStream.h"

#include <vector>

class AudioRenderer;

#define BGM_FADE_INCREMENT 0.1f
#define BGM_FADE_DECREMENT 0.5f

typedef void (*AudioRequestPCMCallback)(void* data, int size, void* meta_data);

struct TrackedAudioHandle
{
private:
	friend class AudioRenderer;

	int m_index;
	int m_slot;

public:
	int Get_Index() { return m_index; }
	int Get_Slot()  { return m_slot; }

	SoundChannel* Get_Channel();
	void Dispose();

};

struct TrackedAudio
{
public:
	bool			Is_Free;
	int				Index;
	double			Start_Time;
	SoundHandle*	Sfx;
	SoundChannel*	Channel;
	double			Demo_Free_Timer;
};

class AudioRenderer : public Singleton<AudioRenderer>
{
	MEMORY_ALLOCATOR(AudioRenderer, "Audio");

private:
	struct BGMEntry
	{
		SoundChannel* Channel;
		SoundHandle*  Handle;
		float		  Volume;
		float		  SmoothedVolume;
	};

	std::vector<BGMEntry> m_bgm_channels;
	std::vector<SoundHandle*> m_bgm_stack;

	float m_bgm_volume;
	float m_sfx_volume;

	float m_bgm_multiplier;

	enum 
	{
		max_tracked_audio = 64, // Make sure audio_max_channels is > this or we will end up having a lot of channel fighting.
	};

	TrackedAudio m_tracked[max_tracked_audio];
	int m_tracked_index;

	TrackedAudioHandle Allocate_Handle();

public:
	friend struct TrackedAudioHandle;
	
	static AudioRenderer* Create();

	AudioRenderer();
	virtual ~AudioRenderer() {}

	// Base functions.	
	virtual void Tick(const FrameTime& time);
	
	// Create stream buffer.
	virtual void Set_Listener_Count(int count) = 0;
	virtual void Set_Listener_Position(int index, Vector3 position) = 0;
	virtual Sound* Create_Streaming_Sound(int sample_rate, int channels, SoundFormat::Type format, AudioRequestPCMCallback callback, void* meta_data) = 0;

	// Volume functions.
	float Get_BGM_Volume();
	void Set_BGM_Volume(float volume);
	float Get_SFX_Volume();
	void Set_SFX_Volume(float volume);

	float Get_BGM_Volume_Multiplier();
	void Set_BGM_Volume_Multiplier(float volume);

	// BGM Functions.
	void Push_BGM(SoundHandle* sound);
	void Push_BGM(const char* bgm_name);
	void Pop_BGM ();
	SoundChannel* Get_Active_BGM_Channel();
	int Get_BGM_Stack_Count();

	// Tracked handles.
	void Stop_All_Tracked();
	TrackedAudioHandle Play_Tracked(SoundHandle* sound, bool start_paused);
	TrackedAudioHandle Play_Tracked(SoundHandle* sound, bool start_paused, Vector3 position);
	TrackedAudioHandle Get_Tracked_Audio_Handle(int index, int slot);

	void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta, bool bScrubbing);


};

#endif

