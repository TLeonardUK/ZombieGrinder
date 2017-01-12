// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_GAMEMODETEAM_
#define _GAME_GAMEMODETEAM_

#include "Generic/Types/Color.h"

#include "Generic/Patterns/Singleton.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Engine/GameRunner.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

class GameModeTeam
{
	MEMORY_ALLOCATOR(GameModeTeam, "Game");

public:
	std::string			Name;
	Color				Primary_Color;

	CVMGCRoot			Script_Handle;

};

#endif

