// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Map/MapSerializer.h"
#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Blocks/MapFilePreviewBlock.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Runner/Game.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Generic/Threads/Thread.h"

MapSerializer::MapSerializer(Map* map)
	: m_map(map)
{
}

void MapSerializer::Serialize(Stream* stream)
{
	// Create a map file.
	Map* map = Game::Get()->Get_Map();
	MapFile* map_file = new MapFile();
	GameScene* scene = Game::Get()->Get_Game_Scene();
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	// Add header block.
	MapFileHeaderBlock* header_block = map_file->New_Block<MapFileHeaderBlock>();
	*header_block = map->Get_Map_Header();

	// Add preview block.
	MapFilePreviewBlock* preview_block = map_file->New_Block<MapFilePreviewBlock>();
	preview_block->Preview = map->Get_Preview_Pixmap();

	// Add layers.
	for (int i = 0; i < map->Get_Depth(); i++)
	{
		MapLayer* layer = map->Get_Layer(i);

		MapFileLayerBlock* layer_block = map_file->New_Block<MapFileLayerBlock>();
		layer_block->Width	= map->Get_Width();
		layer_block->Height	= map->Get_Height();
		layer_block->Tiles  = layer->Get_Tiles();
	}

	// Add all serializable objects.
	std::vector<Actor*> actors = scene->Get_Actors();
	for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = dynamic_cast<ScriptedActor*>(*iter);
		if (actor != NULL)
		{
			CVMLinkedSymbol* symbol = actor->Get_Script_Object().Get()->Get_Symbol();

			if (symbol->symbol->Get_Meta_Data<bool>("Placeable", false) == true)
			{
				// Save object block.
				MapFileObjectBlock* object_block = map_file->New_Block<MapFileObjectBlock>();
				object_block->Class_Name = symbol->symbol->name;

				BinaryStream bs;
				int output_version = 0;
				CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());
				actor->Get_Script_Object().Get()->Serialize(&bs, CVMObjectSerializeFlags::Full, &output_version);		
				object_block->Serialized_Data_Version = output_version;
				object_block->Serialized_Data.Set(bs.Data(), (int)bs.Length());				
				bs.Close();
			}
		}		
	}

	// Save output map file.
	map_file->Save(stream);	
}

void MapSerializer::Serialize(const char* path)
{
	Stream* stream = StreamFactory::Open(path, (StreamMode::Type)(StreamMode::Write | StreamMode::Truncate));
	if (stream == NULL)
	{
		return;
	}
	Serialize(stream);
	SAFE_DELETE(stream);
}