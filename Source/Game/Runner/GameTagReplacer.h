// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_TAG_REPLACER_
#define _GAME_TAG_REPLACER_

#include "Engine/Renderer/Text/TagReplacer.h"

class GameTagReplacer : public TagReplacer 
{
	MEMORY_ALLOCATOR(GameTagReplacer, "Game");

private:
	void Do_Replace(std::string& buffer);

};

#endif

