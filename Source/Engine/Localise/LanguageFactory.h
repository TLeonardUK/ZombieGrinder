// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LANGUAGEFACTORY_
#define _ENGINE_LANGUAGEFACTORY_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Engine/IO/Stream.h"

#include "Engine/Localise/Language.h"
#include "Engine/Localise/LanguageHandle.h"

class LanguageFactory
{
	MEMORY_ALLOCATOR(LanguageFactory, "Localise"); 

private:
	static ThreadSafeHashTable<LanguageHandle*, int> m_loaded_banks;

protected:
	static Language*		Try_Load(const char* url);

public:
	
	static std::vector<LanguageHandle*> Get_Languages();

	// Dispose.
	static void Dispose();
	
	// Static methods.
	static LanguageHandle* Load(const char* url);
	static Language*	Load_Without_Handle(const char* url);

};

#endif

