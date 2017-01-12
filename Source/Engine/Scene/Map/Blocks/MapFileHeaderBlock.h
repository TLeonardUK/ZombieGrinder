// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_MAP_MAPFILE_HEADER_BLOCK_
#define _ENGINE_SCENE_MAP_MAPFILE_HEADER_BLOCK_

#include "Engine/Scene/Map/Blocks/MapFileBlock.h"

class SoundHandle;
class TextureHandle;
class AtlasHandle;

#include <string>

struct MapFileHeaderBlock : public MapFileBlock 
{
public:
	std::string		GUID;
	std::string		Short_Name;
	std::string		DEPRECATED_Override_Name;	// Unserialized.
	std::string		Author;
	std::string		Long_Name;
	std::string		Description;
	std::string		Loading_Slogan;

	std::string		Intro_Cutscene;
	std::string		Outro_Cutscene;

	int				Min_Players;
	int				Max_Players;

	std::string		Music_Path;
	SoundHandle*	Music;

	std::string		Preview_Path;
	//TextureHandle*	Preview_Image;

	std::string		Tileset_Name;
	AtlasHandle*	Tileset;

	u64				Workshop_ID;

	int				Width;
	int				Height;
	
public:
	MapFileHeaderBlock();
	bool Decode(MapVersion::Type version, Stream* stream);
	bool Encode(MapVersion::Type version, Stream* stream);
	u32  Get_ID();

};

#endif

