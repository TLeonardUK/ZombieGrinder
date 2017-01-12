// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Audio/Sounds/SoundFactory.h"
#include "Engine/Audio/Sounds/Sound.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/Resources/Compiled/Audio/CompiledAudio.h"

#include "Engine/Resources/ResourceFactory.h"

LinkedList<SoundFactory*>		SoundFactory::m_factories;
ThreadSafeHashTable<SoundHandle*, int>	SoundFactory::m_loaded_sounds;

SoundHandle* SoundFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_sounds.Contains(url_hash))
	{
		DBG_LOG("Loaded sound from cache: %s", url);
		return m_loaded_sounds.Get(url_hash);
	}

	// Try and load texture!
	Sound* sound = Load_Without_Handle(url);
	if (sound != NULL)
	{		
		SoundHandle* handle = new SoundHandle(url, sound);

		m_loaded_sounds.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Sound* SoundFactory::Load_Without_Handle(const char* url)
{
	Stream* chunk_stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	if (chunk_stream == NULL)
	{
		return NULL;
	}

	PatchedBinaryStream* stream = new PatchedBinaryStream(chunk_stream, StreamMode::Read);
	chunk_stream->Close();
	SAFE_DELETE(chunk_stream);

	CompiledAudioSound* header = (CompiledAudioSound*)stream->Take_Data();

	Sound* sound = NULL;

	for (LinkedList<SoundFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		SoundFactory* factory = *iter;
		Sound* result = factory->Try_Load(header);
		if (result != NULL)
		{
			sound = result;
			break;
		}
	}

	if (sound == NULL)
	{
		DBG_LOG("Failed to load sound: %s", url);
		SAFE_DELETE(header);
	}
	else
	{
		DBG_LOG("Loaded sound: %s", url);
	}

	stream->Close();
	SAFE_DELETE(stream);

	return sound;
}

SoundFactory::SoundFactory()
{
	m_factories.Add(this);
}

SoundFactory::~SoundFactory()
{
	m_factories.Remove(m_factories.Find(this));
}

void SoundFactory::Dispose()
{
	for (ThreadSafeHashTable<SoundHandle*, int>::Iterator iter = m_loaded_sounds.Begin(); iter != m_loaded_sounds.End(); iter++)
	{
		SoundHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_sounds.Clear();
}
