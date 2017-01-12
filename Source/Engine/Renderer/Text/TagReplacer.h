// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_TAGREPLACER_
#define _ENGINE_RENDERER_TEXT_TAGREPLACER_

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class TagReplacer
{
	MEMORY_ALLOCATOR(TagReplacer, "Rendering");

private:
	static std::vector<TagReplacer*> m_replacers;

protected:
	virtual void Do_Replace(std::string& buffer) = 0;
	void Inplace_Replace(const char** replacements, std::string& buffer);

public:
	TagReplacer();
	~TagReplacer();

	static std::string Replace(const char* buffer);

};

#endif

