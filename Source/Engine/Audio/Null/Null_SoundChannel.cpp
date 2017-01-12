// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Audio/Null/Null_SoundChannel.h"

Null_SoundChannel::Null_SoundChannel(Null_AudioRenderer* renderer)
	: m_renderer(renderer)
	, m_volume(1.0f)
	, m_position(0)
	, m_pan(0.0f)
	, m_rate(1.0f)
	, m_paused(false)
	, m_muted(false)
{
}

Null_SoundChannel::~Null_SoundChannel()
{
}

void Null_SoundChannel::Set_Spatial_Position(Vector3 volume)
{
}

Vector3 Null_SoundChannel::Get_Spatial_Position()
{
	return Vector3(0.0f, 0.0f, 0.0f);
}

void Null_SoundChannel::Set_Volume(float volume)
{
	m_volume = volume;
}

float Null_SoundChannel::Get_Volume()
{
	return m_volume;
}

void Null_SoundChannel::Set_Position(float position)
{
	m_position = position;
}

float Null_SoundChannel::Get_Position()
{	
	return m_position;
}

void Null_SoundChannel::Set_Pan(float pan)
{
	m_pan = pan;
}

float Null_SoundChannel::Get_Pan()
{
	return m_pan;
}
	
void Null_SoundChannel::Set_Rate(float rate)
{
	m_rate = rate;
}

float Null_SoundChannel::Get_Rate()
{
	return m_rate;
}

bool Null_SoundChannel::Is_Paused()
{
	return m_paused;
}

bool Null_SoundChannel::Is_Playing()
{
	return true;
}

bool Null_SoundChannel::Is_Muted()
{
	return m_muted;
}

void Null_SoundChannel::Pause()
{
	m_paused = true;
}

void Null_SoundChannel::Resume()
{
	m_paused = false;
}

void Null_SoundChannel::Mute()
{
	m_muted = true;
}

void Null_SoundChannel::Unmute()
{
	m_muted = false;
}

