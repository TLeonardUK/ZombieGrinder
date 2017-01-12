// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/Sounds/SoundHandle.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Textures/TextureFactory.h"

#include "Engine/IO/Stream.h"

MapFileHeaderBlock::MapFileHeaderBlock()
	: Workshop_ID(0)
{
}

bool MapFileHeaderBlock::Decode(MapVersion::Type version, Stream* stream)
{
	Short_Name		= stream->Read<std::string>();
	Author			= stream->Read<std::string>();
	Long_Name		= stream->Read<std::string>();
	Description		= stream->Read<std::string>();
	Loading_Slogan	= stream->Read<std::string>();

	Min_Players		= stream->Read<int>();
	Max_Players		= stream->Read<int>();

	Music_Path		= stream->Read<std::string>();
	Music			= ResourceFactory::Get()->Get_Sound(Music_Path.c_str());

	Preview_Path	= stream->Read<std::string>();
	//Preview_Image	= TextureFactory::Load(Preview_Path.c_str(), TextureFlags::NONE);

	Tileset_Name	= stream->Read<std::string>();
	Tileset			= ResourceFactory::Get()->Get_Atlas(Tileset_Name.c_str());

	Width			= stream->Read<int>();
	Height			= stream->Read<int>();

	Workshop_ID			= stream->Read<u64>();
	GUID				= stream->Read<std::string>();

	if (version >= MapVersion::Cutscenes_Added)
	{
		Intro_Cutscene	= stream->Read<std::string>();
		Outro_Cutscene	= stream->Read<std::string>();
	}
	else
	{
		Intro_Cutscene = "";
		Outro_Cutscene = "";
	}

	if (Tileset == NULL)
		DBG_LOG("Map contains unknown tileset '%s'.", Tileset_Name.c_str());

	return true;
}

bool MapFileHeaderBlock::Encode(MapVersion::Type version, Stream* stream)
{
	stream->Write<std::string>(Short_Name);
	stream->Write<std::string>(Author);
	stream->Write<std::string>(Long_Name);
	stream->Write<std::string>(Description);
	stream->Write<std::string>(Loading_Slogan);

	stream->Write<int>		  (Min_Players);
	stream->Write<int>		  (Max_Players);

	stream->Write<std::string>(Music_Path);
	stream->Write<std::string>(Preview_Path);
	stream->Write<std::string>(Tileset_Name);
	
	stream->Write<int>		  (Width);
	stream->Write<int>		  (Height);

	stream->Write<u64>		  (Workshop_ID);

	// Regenerate the UUID each time we save!
	GUID = Platform::Get()->Generate_UUID();
	stream->Write<std::string>(GUID);

	stream->Write<std::string>(Intro_Cutscene);
	stream->Write<std::string>(Outro_Cutscene);

	return true;
}

u32 MapFileHeaderBlock::Get_ID()
{
	return COMBINE_MAP_FILE_BLOCK_ID('H', 'E', 'A', 'D');
}

