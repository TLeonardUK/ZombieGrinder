// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_BANKS_AUDIOBANKFACTORY_
#define _ENGINE_AUDIO_BANKS_AUDIOBANKFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/HashTable.h"
#include "Engine/IO/Stream.h"

#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"

#include "Engine/Audio/Banks/AudioBank.h"
#include "Engine/Audio/Banks/AudioBankHandle.h"

class AudioBankFactory
{
	MEMORY_ALLOCATOR(AudioBankFactory, "Audio"); 

private:
	static HashTable<AudioBankHandle*, int> m_loaded_banks;

protected:
	static AudioBank*		Try_Load(const char* url);

public:
	
	// Dispose.
	static void Dispose();
	
	// Static methods.
	static AudioBankHandle* Load(const char* url);
	static AudioBank*		Load_Without_Handle(const char* url);

};

#endif

