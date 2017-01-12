// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Runner/GameTagReplacer.h"

#include "Game/Version.h"

void GameTagReplacer::Do_Replace(std::string& buffer)
{
	const char* replacements[] = 
	{
		// From						To
		"{GAME_VERSION}",			VersionInfo::FULLVERSION_STRING,
		NULL,						NULL
	};

	TagReplacer::Inplace_Replace(replacements, buffer);
}


