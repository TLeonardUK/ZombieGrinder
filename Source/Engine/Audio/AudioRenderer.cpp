// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Generic/Math/Math.h"

#include "Generic/Stats/Stats.h"

#include "Engine/Audio/Null/Null_AudioRenderer.h"

#ifndef DEDICATED_SERVER_BUILD
#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include "Engine/Audio/FMod/FMod_AudioRenderer.h"
#endif
#endif

#include "Engine/Engine/EngineOptions.h"

DEFINE_FRAME_STATISTIC("Audio/BGM Stack Size", int, g_audio_bgm_stack_size, true);
DEFINE_FRAME_STATISTIC("Audio/BGM Volume", float, g_audio_bgm_volume, true);
DEFINE_FRAME_STATISTIC("Audio/BGM Volume Multiplier", float, g_audio_bgm_volume_multilpier, true);
DEFINE_FRAME_STATISTIC("Audio/Active Tracked Audio", int, g_audio_active_tracked_audio, true);

AudioRenderer* AudioRenderer::Create()
{
#ifdef DEDICATED_SERVER_BUILD
	return new Null_AudioRenderer();
#else

	// If we are not running with GUI we just return a
	// dummny renderer.
	if (*EngineOptions::nogui)
	{
		return new Null_AudioRenderer();
	}

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
	return new FMod_AudioRenderer();
#else
	#error "Platform unsupported."
#endif
#endif
}

AudioRenderer::AudioRenderer()
	: m_bgm_volume(1.0f)
	, m_sfx_volume(1.0f)
	, m_tracked_index(0)
	, m_bgm_multiplier(1.0f)
{
	for (int i = 0; i < max_tracked_audio; i++)
	{
		m_tracked[i].Is_Free = true;
	}
}

void AudioRenderer::Tick(const FrameTime& time)
{
	float increment = BGM_FADE_INCREMENT * time.Get_Delta();
	float decrement = BGM_FADE_DECREMENT * time.Get_Delta();

	/*
	struct unique_audio
	{
		SoundHandle* sfx;
		int count;
	};

	std::vector<unique_audio> unique;
	*/

	// Update tracked audio.
	int total_active_tracked_audio = 0;
	for (int i = 0; i < max_tracked_audio; i++)
	{
		if (m_tracked[i].Is_Free == false && 
			!m_tracked[i].Channel->Is_Playing() && 
			!m_tracked[i].Channel->Is_Paused())
		{
			SAFE_DELETE(m_tracked[i].Channel);
			m_tracked[i].Is_Free = true;
		}
		/*
		elseif (m_tracked[i].Is_Free == false)
		{
			bool found = false;
			for (std::vector<unique_audio>::iterator iter = unique.begin(); iter != unique.end(); iter++)
			{
				unique_audio& audio = *iter;
				if (audio.sfx == m_tracked[i].Sfx)
				{
					audio.count++;
					found = true;
					break;
				}
			}

			if (!found)
			{
				unique_audio audio;
				audio.sfx = m_tracked[i].Sfx;
				audio.count = 1;
   				unique.push_back(audio);
			}
		}
		*/

		if (m_tracked[i].Is_Free == false)
		{
			total_active_tracked_audio++;
		}
	}

	/*
	DBG_LOG("========== TRACKED AUDIO =============");
	for (std::vector<unique_audio>::iterator iter = unique.begin(); iter != unique.end(); iter++)
	{
		unique_audio& audio = *iter;
		DBG_LOG("%i: %s", audio.count, audio.sfx->Get()->Get_Name().c_str());
	}
	*/

	// Update sounds.
	if (m_bgm_stack.size() > 0)
	{
		SoundHandle* active_bgm = m_bgm_stack.at(m_bgm_stack.size() - 1);
		bool bFoundChannel = false;

		for (int i = 0; i < (int)m_bgm_channels.size(); i++)
		{
			BGMEntry& entry = m_bgm_channels[i];

			bool active = (entry.Handle == active_bgm);

			if (active == true)
			{
				entry.Volume = Min(entry.Volume + increment, 1.0f);
				bFoundChannel = true;
			}
			else
			{
				entry.Volume = Max(entry.Volume - decrement, 0.0f);
			}

			if (entry.Channel != NULL)
			{
				bool bNew = false;
				if (!entry.Channel->Is_Playing())
				{
					DBG_LOG("Had to restart music channel. Appeared to have stopped playing (stolen?)");
					SAFE_DELETE(entry.Channel);
					entry.Channel = active_bgm->Get()->Allocate_Channel(true);
					entry.Handle->Get()->Play(entry.Channel, false);
					bNew = true;
				}
				entry.SmoothedVolume = Math::Lerp(entry.SmoothedVolume, entry.Volume * m_bgm_volume * m_bgm_multiplier, 1.0f * time.Get_Delta_Seconds());
				entry.Channel->Set_Volume(entry.SmoothedVolume);
				if (bNew)
				{
					entry.Channel->Resume();
				}
			}
		}

		if (bFoundChannel == false && active_bgm != NULL)
		{
			BGMEntry entry;
			entry.Channel = active_bgm->Get()->Allocate_Channel(true);
			entry.Volume = 0.0f;
			entry.SmoothedVolume = 0.0f;
			entry.Handle = active_bgm;

			active_bgm->Get()->Play(entry.Channel, true);
			entry.Channel->Set_Volume(0.0f);
			entry.Channel->Resume();

			m_bgm_channels.push_back(entry);

			DBG_LOG("Adding new BGM channel for sound '%s'.", active_bgm->Get()->Get_Name().c_str());
		}
	}

	g_audio_active_tracked_audio.Set(total_active_tracked_audio);
	g_audio_bgm_stack_size.Set(m_bgm_stack.size());
	g_audio_bgm_volume.Set(m_bgm_volume);
	g_audio_bgm_volume_multilpier.Set(m_bgm_multiplier);
}

void AudioRenderer::Push_BGM(const char* bgm_name)
{
	SoundHandle* sound = ResourceFactory::Get()->Get_Sound(bgm_name);
	Push_BGM(sound);
}

void AudioRenderer::Push_BGM(SoundHandle* sound)
{
	DBG_LOG("Pushing BGM: %s, now %i", sound->Get()->Get_Name().c_str(), m_bgm_stack.size() + 1);

	m_bgm_stack.push_back(sound);

	/*
	BGMEntry entry;
	if (sound != NULL)
	{
		entry.Channel = sound->Get()->Allocate_Channel();
	}
	else
	{
		entry.Channel = NULL;
	}
	entry.Volume = 0.0f;
	if (sound != NULL)
	{
		sound->Get()->Play(entry.Channel, true);
		entry.Channel->Set_Volume(0.0f);
		entry.Channel->Resume();
	}

	m_bgm_stack.push_back(entry);
	*/
}

void AudioRenderer::Pop_BGM()
{
	DBG_ASSERT_STR(m_bgm_stack.size() > 0, "Attempt to pop bgm off empty stack.");

	m_bgm_stack.pop_back();

	/*
	BGMEntry& entry = m_bgm_stack.at(m_bgm_stack.size() - 1);
	m_bgm_stack.pop_back();
	m_fade_stack.push_back(entry);
	*/

	DBG_LOG("Poping BGM. Now %i entries.", m_bgm_stack.size());
}

int  AudioRenderer::Get_BGM_Stack_Count()
{
	return m_bgm_stack.size();
}

SoundChannel* AudioRenderer::Get_Active_BGM_Channel()
{	
	if (m_bgm_stack.size() > 0)
	{
		SoundHandle* active_bgm = m_bgm_stack.at(m_bgm_stack.size() - 1);

		for (int i = 0; i < (int)m_bgm_channels.size(); i++)
		{
			BGMEntry& entry = m_bgm_channels[i];

			if (entry.Handle == active_bgm)
			{
				return entry.Channel;
			}
		}
	}

	return NULL;
}

float AudioRenderer::Get_BGM_Volume()
{
	return m_bgm_volume;
}

void AudioRenderer::Set_BGM_Volume(float volume)
{
	m_bgm_volume = volume;
}

float AudioRenderer::Get_BGM_Volume_Multiplier()
{
	return m_bgm_multiplier;
}

void AudioRenderer::Set_BGM_Volume_Multiplier(float volume)
{
	m_bgm_multiplier = volume;
}

float AudioRenderer::Get_SFX_Volume()
{
	return m_sfx_volume;
}

void AudioRenderer::Set_SFX_Volume(float volume)
{
	m_sfx_volume = volume;
}

TrackedAudioHandle AudioRenderer::Allocate_Handle()
{
	int oldest_slot = -1;
	double oldest_slot_time = 0;

	for (int slot = 0; slot < max_tracked_audio; slot++)
	{
		if (m_tracked[slot].Is_Free == true)
		{
			oldest_slot = slot;
			break;
		}
		else
		{
			if (oldest_slot == -1 || 
				m_tracked[slot].Start_Time < oldest_slot_time)
			{
				oldest_slot_time = m_tracked[slot].Start_Time;
				oldest_slot = slot;
			}
		}
	}

	if (m_tracked[oldest_slot].Is_Free == false)
	{
		SAFE_DELETE(m_tracked[oldest_slot].Channel);
	}

	TrackedAudioHandle handle;
	handle.m_index = m_tracked_index++;
	handle.m_slot  = oldest_slot;

	return handle;
}

void AudioRenderer::Stop_All_Tracked()
{
	for (int i = 0; i < max_tracked_audio; i++)
	{
		if (m_tracked[i].Is_Free == false)
		{
			SAFE_DELETE(m_tracked[i].Channel);
			m_tracked[i].Is_Free = true;
		}
	}
}

void AudioRenderer::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta, bool bScrubbing)
{
	for (int i = 0; i < max_tracked_audio; i++)
	{
		TrackedAudio& audio = m_tracked[i];

		int name_hash = (!audio.Is_Free && audio.Sfx) ? audio.Sfx->Get()->Get_Name_Hash() : 0;
		bool is_free = audio.Is_Free;

		Vector3 spatial_position;
		float volume = 1.0f;
		float rate = 1.0f;
		bool is_muted = false;
		bool is_paused = false;

		if (!is_free && audio.Channel && bSaving)
		{
			spatial_position = audio.Channel->Get_Spatial_Position();
			volume = audio.Channel->Get_Volume();
			rate = audio.Channel->Get_Rate();
			is_muted = audio.Channel->Is_Muted();
			is_paused = audio.Channel->Is_Paused();
		}

		stream->Serialize<bool>(bSaving, is_free);
		stream->Serialize<int>(bSaving, name_hash);
		stream->Serialize<float>(bSaving, spatial_position.X);
		stream->Serialize<float>(bSaving, spatial_position.Y);
		stream->Serialize<float>(bSaving, spatial_position.Z);
		stream->Serialize<float>(bSaving, volume);
		stream->Serialize<float>(bSaving, rate);
		stream->Serialize<bool>(bSaving, is_muted);
		stream->Serialize<bool>(bSaving, is_paused);

		if (!bSaving && !bScrubbing)
		{
			// Dispose of old audio.
			if (!audio.Is_Free && is_free)
			{
				//double elapsed = Platform::Get()->Get_Ticks() - audio.Demo_Free_Timer;

				//if (elapsed > 50.0f) // We delay the free for a little bit of time to ensure sound has finished, as there many be slight latency
									 // on different systems.
				{
					SAFE_DELETE(audio.Channel);
					audio.Is_Free = true;
				}
			}
			else
			{
				audio.Demo_Free_Timer = Platform::Get()->Get_Ticks();
			}

			// Create new audio.
			if (!is_free)
			{
				SoundHandle* sound = ResourceFactory::Get()->Get_Sound(name_hash);

				if (!audio.Is_Free)
				{
					if (audio.Sfx != sound)
					{
						audio.Is_Free = true;
						SAFE_DELETE(audio.Channel);
					}
				}

				if (audio.Is_Free)
				{
					SoundChannel* channel = sound->Get()->Allocate_Channel();
					sound->Get()->Play(channel, is_paused);

					audio.Is_Free = false;
					audio.Index = m_tracked_index++;
					audio.Channel = channel;
					audio.Sfx = sound;
					audio.Start_Time = Platform::Get()->Get_Ticks();

					//DBG_LOG("[Demo] Starting new tracked audio '%s'.", sound->Get()->Get_Name().c_str());
				}
			}
		}
	}
}

TrackedAudioHandle AudioRenderer::Play_Tracked(SoundHandle* sound, bool start_paused)
{
	TrackedAudioHandle handle = Allocate_Handle();

	SoundChannel* channel = sound->Get()->Allocate_Channel();
	sound->Get()->Play(channel, start_paused);

	m_tracked[handle.m_slot].Is_Free = false;
	m_tracked[handle.m_slot].Index = handle.m_index;
	m_tracked[handle.m_slot].Channel = channel;
	m_tracked[handle.m_slot].Sfx = sound;
	m_tracked[handle.m_slot].Start_Time = Platform::Get()->Get_Ticks();

	return handle;
}

TrackedAudioHandle AudioRenderer::Play_Tracked(SoundHandle* sound, bool start_paused, Vector3 position)
{
	TrackedAudioHandle handle = Allocate_Handle();

	SoundChannel* channel = sound->Get()->Allocate_Channel();
	sound->Get()->Play(channel, position, start_paused);

	m_tracked[handle.m_slot].Is_Free = false;
	m_tracked[handle.m_slot].Index = handle.m_index;
	m_tracked[handle.m_slot].Channel = channel;
	m_tracked[handle.m_slot].Sfx = sound;
	m_tracked[handle.m_slot].Start_Time = Platform::Get()->Get_Ticks();

	return handle;
}

TrackedAudioHandle AudioRenderer::Get_Tracked_Audio_Handle(int index, int slot)
{
	TrackedAudioHandle handle;
	handle.m_slot = slot;
	handle.m_index = index;
	return handle;
}

SoundChannel* TrackedAudioHandle::Get_Channel()
{
	AudioRenderer* renderer = AudioRenderer::Get();
	if (renderer->m_tracked[m_slot].Is_Free == true ||
		renderer->m_tracked[m_slot].Index != m_index)
	{
		return NULL;
	}

	return renderer->m_tracked[m_slot].Channel;
}

void TrackedAudioHandle::Dispose()
{
	AudioRenderer* renderer = AudioRenderer::Get();

	if (renderer->m_tracked[m_slot].Index != m_index)
	{
		return;
	}

	SAFE_DELETE(renderer->m_tracked[m_slot].Channel);
	renderer->m_tracked[m_slot].Is_Free = true;
}

