// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LOCALISE_LANGUAGEHANDLE_
#define _ENGINE_LOCALISE_LANGUAGEHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Localise/Language.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

// The language handle wraps a audio bank class instance, and automatically
// reloads the audio bank if the source file is changed.

class LanguageFactory;
class Language;

class LanguageHandle : public Reloadable
{
	MEMORY_ALLOCATOR(LanguageHandle, "Localise"); 

private:

	Language*			m_bank;
	std::string			m_url;

protected:
	
	friend class LanguageFactory;

	// Only texture factory should be able to modify these!
	LanguageHandle(const char* url, Language* sound);
	~LanguageHandle();
	
	void Reload();

public:

	// Get/Set
	Language* Get();
	std::string Get_URL();

};

#endif

