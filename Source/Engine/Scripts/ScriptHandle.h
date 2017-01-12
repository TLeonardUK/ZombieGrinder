// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCRIPTHANDLE_
#define _ENGINE_SCRIPTHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

class ScriptFactory;
class Script;

class ScriptHandle : public Reloadable
{
	MEMORY_ALLOCATOR(ScriptHandle, "Scripts");

private:

	Script*				m_script;
	std::string			m_url;

protected:
	
	friend class ScriptFactory;

	// Only texture factory should be able to modify these!
	ScriptHandle(const char* url, Script* layout);
	~ScriptHandle();
	
	void Reload();

public:

	// Get/Set
	Script* Get();
	std::string Get_URL();

};

#endif

