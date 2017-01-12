// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Engine/EngineTagReplacer.h"

#include "Engine/Version.h"

void EngineTagReplacer::Do_Replace(std::string& buffer)
{
	const char* replacements[] = 
	{
		// From						To
		"{ENGINE_VERSION}",			EngineAutoVersion::FULLVERSION_STRING,
		NULL,						NULL
	};

	TagReplacer::Inplace_Replace(replacements, buffer);
}


