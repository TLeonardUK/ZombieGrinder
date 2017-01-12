// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Audio/Null/Null_AudioRenderer.h"
#include "Engine/Audio/Null/Null_Sound.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include "Generic/Math/Math.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Engine/EngineOptions.h"

Null_AudioRenderer::Null_AudioRenderer()
{
}

Null_AudioRenderer::~Null_AudioRenderer()
{
}

void Null_AudioRenderer::Tick(const FrameTime& time)
{
	AudioRenderer::Tick(time);
}

void Null_AudioRenderer::Set_Listener_Count(int count)
{
}

void Null_AudioRenderer::Set_Listener_Position(int index, Vector3 position)
{
}

Sound* Null_AudioRenderer::Create_Streaming_Sound(int sample_rate, int channels, SoundFormat::Type type, AudioRequestPCMCallback callback, void* meta_data)
{
	return new Null_Sound(this, NULL);
}
