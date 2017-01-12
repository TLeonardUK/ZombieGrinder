// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ENGINE_TAG_REPLACER_
#define _ENGINE_ENGINE_TAG_REPLACER_

#include "Engine/Renderer/Text/TagReplacer.h"

class EngineTagReplacer : public TagReplacer 
{
	MEMORY_ALLOCATOR(EngineTagReplacer, "Engine");

private:
	void Do_Replace(std::string& buffer);

};

#endif

