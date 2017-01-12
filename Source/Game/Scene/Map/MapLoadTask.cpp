// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Map/MapLoadTask.h"
#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Game/Scene/Map/RandGen/MapRandGenManager.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

#include "Game/Runner/Game.h"
#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetClient.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Engine/GameEngine.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Generic/Threads/Thread.h"

MapLoadTask::MapLoadTask(Map* map, MapFileHandle* map_file)
	: m_success(false)
	, m_cancelled(false)
	, m_map(map)
	, m_map_file(map_file)
{
	DBG_LOG("[MapLoadTask] Created");
}

MapLoadTask::~MapLoadTask()
{
	DBG_LOG("[MapLoadTask] Destroyed");
}

bool MapLoadTask::Was_Success()
{
	return m_success;
}

void MapLoadTask::Cancel()
{
	m_cancelled = true;
}

void MapLoadTask::Run()
{
	MapFile* map_file = m_map_file->Get();
	Scene* scene = GameEngine::Get()->Get_Scene();
	GameScene* game_scene = Game::Get()->Get_Game_Scene();

	DBG_LOG("Background map load beginning for map '%s' (guid=%s mod=%i).", map_file->Get_Header()->Short_Name.c_str(), map_file->Get_Header()->GUID.c_str(), map_file->Get_Header()->Workshop_ID);

	// Prevent main thread trying to run VM when we are loading script instances.
	GameEngine::Get()->Get_VM()->Lock();

	// Get the blocks we are interested in.
	MapFileHeaderBlock*				 header	 = map_file->Get_Header();
	MapFilePreviewBlock*			 preview = map_file->Get_Block<MapFilePreviewBlock>();
	std::vector<MapFileLayerBlock*>  layers	 = map_file->Get_Blocks<MapFileLayerBlock>();
	std::vector<MapFileObjectBlock*> objects = map_file->Get_Blocks<MapFileObjectBlock>();

	// Setup the main map settings.
	m_map->Set_Preview_Pixmap(preview == NULL ? NULL : preview->Preview);
	m_map->Set_Tileset(header->Tileset);
	m_map->Set_Map_Header(*header);
	m_map->Reset_Map_IDs();

	RenderPipeline::Get()->Set_Game_Clear_Color(Color::Black);

	// If this map is a rand-gen template, then time to randomise it!
	bool bRandGen = false;

	for (std::vector<MapFileObjectBlock*>::iterator iter = objects.begin(); iter != objects.end(); iter++)
	{
		MapFileObjectBlock* obj = *iter;
		if (obj->Class_Name == "DC_Room_Marker")
		{
			bRandGen = true;
			break;
		}
	}

	// Random generation and not in editor mode, then lets randomise the loading.
	GameNetServerState state;
	if (GameNetManager::Get()->Game_Server() != NULL)
	{
		state = GameNetManager::Get()->Game_Server()->Get_Server_State();
	}
	else
	{
		state = GameNetManager::Get()->Game_Client()->Get_Server_State();
	}

	if (bRandGen == true && state.In_Editor_Mode == false)
	{
		MapRandGenManager manager;
		manager.Generate(m_map, map_file, NetManager::Get()->Get_Current_Map_Seed());
	}

	// Else use standard loading.
	else
	{
		// Copy all layer data over to the map.
		DBG_ASSERT(layers.size() > 0);
		MapFileLayerBlock* archetype_layer = layers.at(0);
		m_map->Resize(archetype_layer->Width, archetype_layer->Height, layers.size());

		int layer_index = 0;
		for (std::vector<MapFileLayerBlock*>::iterator iter = layers.begin(); iter != layers.end(); iter++, layer_index++)
		{
			MapFileLayerBlock* layer = *iter;
			DBG_ASSERT(layer->Width == archetype_layer->Width && layer->Height == archetype_layer->Height);

			for (int y = 0; y < layer->Height; y++)
			{			
				for (int x = 0; x < layer->Width; x++)
				{
					*m_map->Get_Tile(x, y, layer_index) = layer->Tiles[(y * layer->Width) + x];
				}
			}
		}

		// Instantiate all objects.
		for (std::vector<MapFileObjectBlock*>::iterator iter = objects.begin(); iter != objects.end(); iter++)
		{
			MapFileObjectBlock* obj = *iter;

			// Can return NULL if server constructed instance.
			ScriptedActor* scripted = obj->Create_Instance();
			if (scripted != NULL)
			{
				if (!scripted->Get_Script_Object().Get()->Get_Symbol()->symbol->class_data->is_replicated)
					scripted->Set_Map_ID(m_map->Create_Map_ID());

				scene->Add_Actor(scripted);
				scene->Add_Tickable(scripted);
			}
		}

		// Set the scene boundries to the map size.
		AtlasFrame* frame = header->Tileset->Get()->Get_Frame_By_Index(0);
		scene->Set_Boundries(Rect2D(0.0f, 0.0f, archetype_layer->Width * frame->Rect.Width, archetype_layer->Height * frame->Rect.Height));
	}

	// If we are playing back a demo, destroy all demo-relevant actors
	// as we will be using proxies for them.
	if (DemoManager::Get()->Is_Playing())
	{
		std::vector<Actor*> actors = scene->Get_Actors();
		for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
		{
			Actor* actor = *iter;
			if (actor->Is_Relevant_To_Demo())
			{
				scene->Remove_Actor(actor);
				SAFE_DELETE(actor);
			}
		}
	}

	// Mark dirty so it has to regen all its assets.
	m_map->Mark_Dirty();
	m_map->Mark_Collision_Dirty();

	// Run map tick once to get everything warmed up.
	FrameTime* time = GameEngine::Get()->Get_Time();
	m_map->Tick(*time);

	// Main thread is free to run instances again!
	GameEngine::Get()->Get_VM()->Unlock();

	DBG_LOG("Background map load finished.");
	m_success = true;
}

