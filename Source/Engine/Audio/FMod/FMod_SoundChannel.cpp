// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Audio/FMod/FMod_SoundChannel.h"

#include <algorithm>

FMOD_CHANNEL* FMod_SoundChannel::Get_FMod_Channel()
{
	return m_channel;
}

void FMod_SoundChannel::Set_FMod_Sound(FMOD_SOUND* sound)
{
	FMOD_RESULT result = FMOD_Sound_GetDefaults(sound, &m_default_frequency, &m_default_priority);
	DBG_ASSERT(result == FMOD_OK);
}

FMod_SoundChannel::FMod_SoundChannel(FMod_AudioRenderer* renderer, FMOD_CHANNEL* channel)
	: m_renderer(renderer)
	, m_channel(channel)
	, m_pan(0.0f)
{
}

FMod_SoundChannel::~FMod_SoundChannel()
{
	// Get rid of the channel!
	FMOD_Channel_Stop(m_channel);

	// Release ref-counted fmod channel reference!
	m_channel = NULL;
}

void FMod_SoundChannel::Set_FMod_Channel(FMOD_CHANNEL* sound)
{
	m_channel = sound;
}

void FMod_SoundChannel::Set_Spatial_Position(Vector3 position)
{
	FMOD_VECTOR pos = { position.X, position.Y, 0.0f };
	FMOD_Channel_Set3DAttributes(m_channel, &pos, NULL, NULL);
}

Vector3 FMod_SoundChannel::Get_Spatial_Position()
{
	FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f };
	FMOD_Channel_Get3DAttributes(m_channel, &pos, NULL, NULL);

	return Vector3(pos.x, pos.y, pos.z);
}

void FMod_SoundChannel::Set_Volume(float volume)
{
	FMOD_RESULT result = FMOD_Channel_SetVolume(m_channel, volume);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetVolume failed with 0x%08x.", result);
}

float FMod_SoundChannel::Get_Volume()
{
	float volume = 0.0f;

	FMOD_RESULT result = FMOD_Channel_GetVolume(m_channel, &volume);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_GetVolume failed with 0x%08x.", result);

	return volume;
}

void FMod_SoundChannel::Set_Position(float position)
{
	FMOD_RESULT result = FMOD_Channel_SetPosition(m_channel, (unsigned int)position, FMOD_TIMEUNIT_MS);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetPosition failed with 0x%08x.", result);
}

float FMod_SoundChannel::Get_Position()
{	
	unsigned int position = 0;

	FMOD_RESULT result = FMOD_Channel_GetPosition(m_channel, &position, FMOD_TIMEUNIT_MS);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_GetPosition failed with 0x%08x.", result);

	return (float)position;
}

void FMod_SoundChannel::Set_Pan(float pan)
{
	FMOD_RESULT result = FMOD_Channel_SetPan(m_channel, pan);
	m_pan = pan;
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetPan failed with 0x%08x.", result);
}

float FMod_SoundChannel::Get_Pan()
{
	return m_pan;
}
	
void FMod_SoundChannel::Set_Rate(float rate)
{
	FMOD_RESULT result = FMOD_Channel_SetFrequency(m_channel, m_default_frequency * rate);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetFrequency failed with 0x%08x.", result);
}

float FMod_SoundChannel::Get_Rate()
{
	float frequency = 0.0f;

	FMOD_RESULT result = FMOD_Channel_GetFrequency(m_channel, &frequency);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_GetFrequency failed with 0x%08x.", result);

	return frequency / m_default_frequency;
}

bool FMod_SoundChannel::Is_Paused()
{
	FMOD_BOOL paused = false;

	FMOD_RESULT result = FMOD_Channel_GetPaused(m_channel, &paused);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_GetPaused failed with 0x%08x.", result);

	return result == FMOD_OK && paused != 0;
}

bool FMod_SoundChannel::Is_Playing()
{
	FMOD_BOOL playing = false;

	FMOD_RESULT result = FMOD_Channel_IsPlaying(m_channel, &playing);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_IsPlaying failed with 0x%08x.", result);

	return result == FMOD_OK && playing != 0;
}

bool FMod_SoundChannel::Is_Muted()
{
	FMOD_BOOL mute = false;

	FMOD_RESULT result = FMOD_Channel_GetMute(m_channel, &mute);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_GetMute failed with 0x%08x.", result);

	return result == FMOD_OK && mute != 0;
}

void FMod_SoundChannel::Pause()
{
	FMOD_RESULT result = FMOD_Channel_SetPaused(m_channel, true);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetPaused failed with 0x%08x.", result);
}

void FMod_SoundChannel::Resume()
{
	FMOD_RESULT result = FMOD_Channel_SetPaused(m_channel, false);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetPaused failed with 0x%08x.", result);
}

void FMod_SoundChannel::Mute()
{
	FMOD_RESULT result = FMOD_Channel_SetMute(m_channel, true);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetMute failed with 0x%08x.", result);
}

void FMod_SoundChannel::Unmute()
{
	FMOD_RESULT result = FMOD_Channel_SetMute(m_channel, false);
	DBG_ASSERT_STR(result == FMOD_OK || result == FMOD_ERR_CHANNEL_STOLEN || result == FMOD_ERR_INVALID_HANDLE, "FMOD_Channel_SetMute failed with 0x%08x.", result);
}

#endif