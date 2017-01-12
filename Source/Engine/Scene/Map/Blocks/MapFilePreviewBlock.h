// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_MAP_MAPFILE_PREVIEW_BLOCK_
#define _ENGINE_SCENE_MAP_MAPFILE_PREVIEW_BLOCK_

#include "Engine/Scene/Map/Blocks/MapFileBlock.h"

#include "Engine/Scene/Map/Map.h"

class SoundHandle;
class TextureHandle;
class AtlasHandle;

struct MapFilePreviewBlock : public MapFileBlock 
{
public:
	Pixelmap* Preview;

public:
	MapFilePreviewBlock();
	~MapFilePreviewBlock();

	bool Decode(MapVersion::Type version, Stream* stream);
	bool Encode(MapVersion::Type version, Stream* stream);
	u32  Get_ID();

};

#endif

