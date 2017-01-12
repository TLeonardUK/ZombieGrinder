// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_MAP_MAPFILE_LAYER_BLOCK_
#define _GAME_SCENE_MAP_MAPFILE_LAYER_BLOCK_

#include "Engine/Scene/Map/Blocks/MapFileBlock.h"

#include "Engine/Scene/Map/Map.h"

class SoundHandle;
class TextureHandle;
class AtlasHandle;

struct MapFileLayerBlock : public MapFileBlock 
{
public:
	int					Width;
	int					Height;
	MapTile*			Tiles;

public:
	MapFileLayerBlock();
	~MapFileLayerBlock();

	bool Decode(MapVersion::Type version, Stream* stream);
	bool Encode(MapVersion::Type version, Stream* stream);
	u32  Get_ID();

};

#endif

