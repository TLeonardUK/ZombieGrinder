// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_MAP_MAPFILE_BLOCK_FACTORIES_
#define _GAME_SCENE_MAP_MAPFILE_BLOCK_FACTORIES_

#include "Engine/Scene/Map/Blocks/MapFileBlock.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"
#include "Engine/Scene/Map/Blocks/MapFilePreviewBlock.h"

#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"

class MapBlockFactories
{
public:
	DEFINE_MAP_BLOCK_FACTORY(MapFileLayerBlock);
	DEFINE_MAP_BLOCK_FACTORY(MapFileObjectBlock);
	DEFINE_MAP_BLOCK_FACTORY(MapFileHeaderBlock);
	DEFINE_MAP_BLOCK_FACTORY(MapFilePreviewBlock);

public:
	MapBlockFactories()
	{
	}
};

#endif

