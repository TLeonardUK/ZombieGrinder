// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCRIPT_
#define _ENGINE_SCRIPT_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Config/ConfigFile.h"

#include <vector>

class ScriptFactory;
class CVMBinary;

class Script
{
	MEMORY_ALLOCATOR(Script, "Scripts");

private:	
	CVMBinary* m_binary;
	std::string m_name;

protected:
	friend class ScriptHandle;
	friend class ScriptFactory;

	static Script* Load(const char* url);

public:
	Script();
	~Script();

	std::string Get_Name();
	CVMBinary* Get_Binary();

};

#endif

