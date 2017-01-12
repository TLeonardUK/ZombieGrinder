// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCRIPTFACTORY_
#define _ENGINE_SCRIPTFACTORY_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/ThreadSafeHashTable.h"

class Script;
class ScriptHandle;

class ScriptFactory
{
	MEMORY_ALLOCATOR(ScriptFactory, "Scripts");

protected:
	static ThreadSafeHashTable<ScriptHandle*, int> m_loaded_scripts;

	ScriptFactory();
	~ScriptFactory();

public:

	// Disposal.
	static void Dispose();
		
	// Static methods.
	static ScriptHandle* Load				  (const char* url);
	static Script*		 Load_Without_Handle(const char* url);

};

#endif

