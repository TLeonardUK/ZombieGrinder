// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Audio/FMod/FMod_AudioRenderer.h"
#include "Engine/Audio/FMod/FMod_Sound.h"
#include "Engine/Audio/FMod/FMod_SoundChannel.h"

FMod_Sound::FMod_Sound(FMod_AudioRenderer* renderer, FMOD_SOUND* sound, CompiledAudioSound* resource)
	: m_sound(sound)
	, m_renderer(renderer)
	, m_resource(resource)
	, m_name(m_resource->name)
	, m_name_hash(m_resource->name_hash)
{
	// If we are not streaming we no longer need raw resource data.
	if ((m_resource->flags & SoundFlags::Streaming) == 0)
	{
		SAFE_DELETE(m_resource);
	}
}

FMod_Sound::FMod_Sound(FMod_AudioRenderer* renderer, AudioRequestPCMCallback callback, void* callback_metadata)
	: m_renderer(renderer)
	, m_callback(callback)
	, m_callback_metadata(callback_metadata)
	, m_resource(NULL)
	, m_name("")
	, m_name_hash(0)
{
}

void FMod_Sound::Set_Sound(FMOD_SOUND* sound)
{
	m_sound = sound;
}

std::string FMod_Sound::Get_Name() const
{
	return m_name;
}

int FMod_Sound::Get_Name_Hash() const
{
	return m_name_hash;
}

CompiledAudioSound* FMod_Sound::Get_Compiled() const
{
	return m_resource;
}

FMod_Sound::~FMod_Sound()
{
	FMOD_Sound_Release(m_sound);
	SAFE_DELETE(m_resource);
	m_sound = NULL;
}

void FMod_Sound::Play(SoundChannel* channel, bool start_paused) const
{
	FMod_SoundChannel*	fmod_channel = dynamic_cast<FMod_SoundChannel*>(channel);
	FMOD_SYSTEM*		system		 = m_renderer->Get_FMod_System();
	FMOD_CHANNEL*		chan		 = fmod_channel == NULL ? NULL : fmod_channel->Get_FMod_Channel();

	// Stop old channel.
	if (fmod_channel != NULL)
	{
		FMOD_Channel_Stop(chan);
	}

	// Start new channel.
	FMOD_RESULT result = FMOD_System_PlaySound(system, m_sound, NULL, true, &chan);
	if (result != FMOD_OK)
	{
		DBG_LOG("Play fmod sound failed with error %i\n", result);
	}

	// Update channel so it knows its playing this sound.
	if (fmod_channel != NULL)
	{
		fmod_channel->Set_FMod_Sound(m_sound);
		fmod_channel->Set_FMod_Channel(chan);
	}

	// If new channel set SFX volume as default.
	if (chan != NULL)
	{
		result = FMOD_Channel_SetVolume(chan, AudioRenderer::Get()->Get_SFX_Volume());
		DBG_ASSERT(result == FMOD_OK);
	}

	if (start_paused == false)
	{
		FMOD_Channel_SetPaused(chan, false);
	}

	// Set attenuation information for sound.
	FMOD_Channel_Set3DMinMaxDistance(chan, 99999999.0f, 99999999.0f);

	FMOD_Channel_SetUserData(chan, static_cast<Sound*>(const_cast<FMod_Sound*>(this)));
}

void FMod_Sound::Play(SoundChannel* channel, Vector3 position, bool start_paused) const
{
	FMod_SoundChannel*	fmod_channel = dynamic_cast<FMod_SoundChannel*>(channel);
	FMOD_SYSTEM*		system		 = m_renderer->Get_FMod_System();
	FMOD_CHANNEL*		chan		 = fmod_channel == NULL ? NULL : fmod_channel->Get_FMod_Channel();

	// Stop old channel.
	if (fmod_channel != NULL)
	{
		FMOD_Channel_Stop(chan);
	}

	// Start new channel.
	FMOD_RESULT result = FMOD_System_PlaySound(system, m_sound, NULL, true, &chan);
	if (result != FMOD_OK)
	{
		DBG_LOG("Play fmod sound failed with error %i\n", result);
	}

	if (channel != NULL && fmod_channel->Is_Unstealable)
	{
		// Prevent fmod stealing this channel.
		result = FMOD_Channel_SetPriority(chan, 0);
		if (result != FMOD_OK)
		{
			DBG_LOG("Failed to set fmod channel priority with error %i\n", result);
		}
	}

	// Set attenuation information for sound.
	// TODO: Blah need some sane values for this.
	//FMOD_Channel_SetMode(chan, FMOD_3D);

	FMOD_Channel_Set3DMinMaxDistance(chan, 99999999.0f, 99999999.0f);
	FMOD_Channel_Set3DMinMaxDistance(chan, 128.0f, 300.0f);

	FMOD_VECTOR pos = { position.X, position.Y, 0.0f };
	FMOD_Channel_Set3DAttributes(chan, &pos, NULL, NULL);

	FMOD_Channel_SetUserData(chan, static_cast<Sound*>(const_cast<FMod_Sound*>(this)));

	// Update channel so it knows its playing this sound.
	if (fmod_channel != NULL)
	{
		fmod_channel->Set_FMod_Sound(m_sound);
		fmod_channel->Set_FMod_Channel(chan);
	}

	// If new channel set SFX volume as default.
	if (chan != NULL)
	{
		result = FMOD_Channel_SetVolume(chan, AudioRenderer::Get()->Get_SFX_Volume());
		DBG_ASSERT(result == FMOD_OK);
	}

	if (start_paused == false)
	{
		FMOD_Channel_SetPaused(chan, false);
	}
}

SoundChannel* FMod_Sound::Allocate_Channel(bool unstealable) const
{
	FMOD_SYSTEM*  system = m_renderer->Get_FMod_System();
	FMOD_CHANNEL* chan	 = NULL;

	FMOD_RESULT result = FMOD_System_PlaySound(system, m_sound, NULL, true, &chan);
	if (result != FMOD_OK)
	{
		DBG_LOG("Allocate fmod channel failed with error %i\n", result);
		return NULL;
	}

	// Prevent fmod stealing this channel.
	if (unstealable)
	{
		result = FMOD_Channel_SetPriority(chan, 0);
		if (result != FMOD_OK)
		{
			DBG_LOG("Failed to set fmod channel priority with error %i\n", result);
		}
	}

	// If new channel set SFX volume as default.
	if (chan != NULL)
	{
		result = FMOD_Channel_SetVolume(chan, AudioRenderer::Get()->Get_SFX_Volume());
		DBG_ASSERT(result == FMOD_OK);
	}

	// Create actual fmod channel.
	FMod_SoundChannel* fmod_channel = new FMod_SoundChannel(m_renderer, chan);
	fmod_channel->Is_Unstealable = unstealable;
	fmod_channel->Set_FMod_Sound(m_sound);

	FMOD_Channel_SetUserData(chan, static_cast<Sound*>(const_cast<FMod_Sound*>(this)));

	// Return channel instance.
	return fmod_channel;
}

#endif