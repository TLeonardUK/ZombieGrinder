// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorChangeMapStateAction.h"
#include "Game/Game/EditorManager.h"
#include "Game/Runner/Game.h"

#include "Game/Network/ActorReplicator.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/GameScene.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Map/Map.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

EditorChangeMapStateAction::EditorChangeMapStateAction(NetPacket_C2S_EditorChangeMapState* action)
	: m_action(*action)
	, m_map_size_changed(false)
{
}

const char* EditorChangeMapStateAction::Get_Name()
{
	return "Change Map State";
}

void EditorChangeMapStateAction::Undo()
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	NetPacket_C2S_EditorChangeMapState* packet = &m_action;

	DBG_LOG("Performing map properties undo.");
	
	if (packet->Old_Map_Data.Size() > 0)
	{
		Map* map = Game::Get()->Get_Map();

		BinaryStream bs(packet->Old_Map_Data.Buffer(), packet->Old_Map_Data.Size());
		map->Get_Map_Header().Decode(MapVersion::Current_Version, &bs);
		map->Apply_Header_Changes();

		UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
		if (editor != NULL)
		{
			editor->Map_Properties_Changed();
		}

		// Restore map.
		if (m_map_size_changed == true)
		{
			int tile_index = 0;
			for (int z = 0; z < map->Get_Depth(); z++)
			{
				for (int x = 0; x < map->Get_Width(); x++)
				{				
					for (int y = 0; y < map->Get_Height(); y++)
					{
						MapTile& old_tile = m_original_tiles.at(tile_index++);
						old_tile.TileAnimation = NULL;
						*map->Get_Tile(x, y, z) = old_tile; 
					}
				}
				map->Get_Layer(z)->Mark_Dirty();
			}
			m_original_tiles.clear();
		}

		bs.Close();
	}
}

void EditorChangeMapStateAction::Do()
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	NetPacket_C2S_EditorChangeMapState* packet = &m_action;

	DBG_LOG("Performing map properties change.");
	
	if (packet->Map_Data.Size() > 0)
	{
		Map* map = Game::Get()->Get_Map();

		BinaryStream bs(packet->Map_Data.Buffer(), packet->Map_Data.Size());
		map->Get_Map_Header().Decode(MapVersion::Current_Version, &bs);
		
		// Store old tile state before applying (gggrrrrr, dam resizing).		
		m_map_size_changed = (map->Get_Width() != map->Get_Map_Header().Width || map->Get_Height() != map->Get_Map_Header().Height);

		m_original_tiles.reserve((int)(map->Get_Width() * map->Get_Height() * map->Get_Depth()));
		for (int z = 0; z < map->Get_Depth(); z++)
		{
			for (int x = 0; x < map->Get_Width(); x++)
			{				
				for (int y = 0; y < map->Get_Height(); y++)
				{
					MapTile* tile = map->Get_Tile(x, y, z);
					m_original_tiles.push_back(*tile);
				}
			}
		}

		// Appppppply
		map->Apply_Header_Changes();

		UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
		if (editor != NULL)
		{
			editor->Map_Properties_Changed();
		}

		bs.Close();
	}
}
