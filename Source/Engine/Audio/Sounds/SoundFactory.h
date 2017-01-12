// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_SOUNDS_SOUNDFACTORY_
#define _ENGINE_AUDIO_SOUNDS_SOUNDFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Engine/IO/Stream.h"

#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Resources/Compiled/Audio/CompiledAudio.h"

class SoundFactory
{
	MEMORY_ALLOCATOR(SoundFactory, "Audio");

private:
	static LinkedList<SoundFactory*>	m_factories;
	static ThreadSafeHashTable<SoundHandle*, int>	m_loaded_sounds;

public:
	
	// Dispose.
	static void Dispose();
	
	// Static methods.
	static SoundHandle* Load(const char* url);
	static Sound* Load_Without_Handle(const char* url);

	// Constructors
	SoundFactory();	
	virtual ~SoundFactory();	

	// Derived factory methods.
	virtual Sound* Try_Load(CompiledAudioSound* header) = 0;

};

#endif

