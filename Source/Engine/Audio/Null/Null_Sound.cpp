// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Audio/Null/Null_AudioRenderer.h"
#include "Engine/Audio/Null/Null_Sound.h"
#include "Engine/Audio/Null/Null_SoundChannel.h"

Null_Sound::Null_Sound(Null_AudioRenderer* renderer, CompiledAudioSound* resource)
	: m_renderer(renderer)
	, m_resource(resource)
{
}

Null_Sound::~Null_Sound()
{
	SAFE_DELETE(m_resource);
}

void Null_Sound::Play(SoundChannel* channel, bool start_paused) const
{
}

void Null_Sound::Play(SoundChannel* channel, Vector3 position, bool start_paused) const
{
}

CompiledAudioSound* Null_Sound::Get_Compiled() const
{
	return m_resource;
}

std::string Null_Sound::Get_Name() const
{
	return m_resource->name;
}

int Null_Sound::Get_Name_Hash() const
{
	return m_resource->name_hash;
}

SoundChannel* Null_Sound::Allocate_Channel(bool unstealable) const
{
	Null_SoundChannel* null_channel = new Null_SoundChannel(m_renderer);
	return null_channel;
}
