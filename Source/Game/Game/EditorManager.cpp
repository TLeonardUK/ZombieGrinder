// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Game/EditorManager.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetUser.h"

#include "Engine/Network/NetFileTransfer.h"

#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/Atlas.h"

#include "Game/Runner/Game.h"
#include "Game/Scene/GameScene.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"
#include "Game/Scene/Map/MapSerializer.h"
#include "Game/Scene/Map/Blocks/MapFileLayerBlock.h"
#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Network/ActorReplicator.h"

#include "Engine/Particles/ParticleManager.h"

#include "Generic/Math/Random.h"

#include "Game/Scene/Actors/ScriptedActor.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/Editor/Actions/EditorAction.h"
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorTilemapModifyAction.h"
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectRespositionAction.h"
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectDeletionAction.h"
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectCreationAction.h"
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorObjectChangeStateAction.h"
#include "Game/UI/Scenes/Editor/Actions/Scene/EditorChangeMapStateAction.h"

EditorManager::EditorManager()
	: m_callback_client_tilemap_modification_recieved(this, &EditorManager::Callback_Recieve_Client_Tilemap_Modification, false, NULL)
	, m_callback_server_tilemap_modification_recieved(this, &EditorManager::Callback_Recieve_Server_Tilemap_Modification, false, NULL)
	, m_callback_client_object_reposition_recieved(this, &EditorManager::Callback_Recieve_Client_Object_Reposition, false, NULL)
	, m_callback_server_object_reposition_recieved(this, &EditorManager::Callback_Recieve_Server_Object_Reposition, false, NULL)
	, m_callback_client_delete_objects_recieved(this, &EditorManager::Callback_Recieve_Client_Delete_Objects, false, NULL)
	, m_callback_server_delete_objects_recieved(this, &EditorManager::Callback_Recieve_Server_Delete_Objects, false, NULL)
	, m_callback_client_create_objects_recieved(this, &EditorManager::Callback_Recieve_Client_Create_Objects, false, NULL)
	, m_callback_server_create_objects_recieved(this, &EditorManager::Callback_Recieve_Server_Create_Objects, false, NULL)
	, m_callback_client_change_object_recieved(this, &EditorManager::Callback_Recieve_Client_Change_Object, false, NULL)
	, m_callback_server_change_object_recieved(this, &EditorManager::Callback_Recieve_Server_Change_Object, false, NULL)
	, m_callback_client_change_map_recieved(this, &EditorManager::Callback_Recieve_Client_Change_Map, false, NULL)
	, m_callback_server_change_map_recieved(this, &EditorManager::Callback_Recieve_Server_Change_Map, false, NULL)
	, m_callback_client_reset_map_recieved(this, &EditorManager::Callback_Recieve_Client_Reset_Map, false, NULL)
	, m_callback_server_reset_map_recieved(this, &EditorManager::Callback_Recieve_Server_Reset_Map, false, NULL)
	, m_callback_client_undo_request_recieved(this, &EditorManager::Callback_Recieve_Client_Undo_Request, false, NULL)
	, m_callback_client_redo_request_recieved(this, &EditorManager::Callback_Recieve_Client_Redo_Request, false, NULL)
	, m_callback_server_undo_request_recieved(this, &EditorManager::Callback_Recieve_Server_Undo_Request, false, NULL)
	, m_callback_server_redo_request_recieved(this, &EditorManager::Callback_Recieve_Server_Redo_Request, false, NULL) 
	, m_callback_server_open_map_recieved(this, &EditorManager::Callback_Recieve_Server_Open_Map, false, NULL) 
	, m_callback_client_open_map_recieved(this, &EditorManager::Callback_Recieve_Client_Open_Map, false, NULL) 
	, m_editor_action_index(0)
	, m_map_open_queued(false)
{
}

EditorManager::~EditorManager()
{
}

int EditorManager::Get_Editor_Action_Index()
{
	return m_editor_action_index;
}

int EditorManager::Get_Undo_Stack_Size()
{
	return m_undo_stack.size();
}

int EditorManager::Get_Redo_Stack_Size()
{
	return m_redo_stack.size();
}

bool EditorManager::All_Users_In_Game()
{
	std::vector<GameNetUser*> users = GameNetManager::Get()->Get_Game_Net_Users();
	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		if (user->Get_Game_State() != GameNetClient_GameState::InGame &&
			user->Get_Game_State() != GameNetClient_GameState::InCutscene)
		{
			return false;
		}
	}

	return true;
}

bool EditorManager::Callback_Recieve_Server_Tilemap_Modification(NetPacket_C2S_ModifyTilemap * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform the action on server copy of world.
	Perform_Action(packet);

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Tilemap_Modification(NetPacket_C2S_ModifyTilemap * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		Perform_Action(packet);
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Server_Object_Reposition(NetPacket_C2S_EditorRepositionObjects * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform the action on server copy of world.
	Perform_Action(packet);

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Object_Reposition(NetPacket_C2S_EditorRepositionObjects * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		Perform_Action(packet);
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Server_Delete_Objects(NetPacket_C2S_EditorDeleteObjects * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform the action on server copy of world.
	Perform_Action(packet);

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Delete_Objects(NetPacket_C2S_EditorDeleteObjects * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		Perform_Action(packet);
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Server_Create_Objects(NetPacket_C2S_EditorCreateObjects * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform the action on server copy of world.
	Perform_Action(packet);

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Create_Objects(NetPacket_C2S_EditorCreateObjects * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		Perform_Action(packet);
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Server_Change_Object(NetPacket_C2S_EditorChangeObjectState * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform the action on server copy of world.
	Perform_Action(packet);

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Change_Object(NetPacket_C2S_EditorChangeObjectState * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		Perform_Action(packet);
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Server_Change_Map(NetPacket_C2S_EditorChangeMapState * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform the action on server copy of world.
	Perform_Action(packet);

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Change_Map(NetPacket_C2S_EditorChangeMapState * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		Perform_Action(packet);
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Server_Reset_Map(NetPacket_C2S_EditorResetMap * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform the action on server copy of world.
	Perform_Action(packet);

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Reset_Map(NetPacket_C2S_EditorResetMap * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		Perform_Action(packet);
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Server_Undo_Request(NetPacket_C2S_EditorRequestUndo * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform undo.
	Undo();

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Server_Redo_Request(NetPacket_C2S_EditorRequestRedo * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform tilemap modification whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Perform redo.
	Redo();

	// Broadcast action on to other players.
	NetManager::Get()->Server()->Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool EditorManager::Callback_Recieve_Client_Undo_Request(NetPacket_C2S_EditorRequestUndo * packet)
{
	if (NetManager::Get()->Server() == NULL)
	{
		Undo();
	}

	m_editor_action_index++;

	return true;
}

bool EditorManager::Callback_Recieve_Client_Redo_Request(NetPacket_C2S_EditorRequestRedo * packet)
{
	if (NetManager::Get()->Server() == NULL)
	{
		Redo();
	}

	m_editor_action_index++;

	return true;
}

void EditorManager::Perform_Action(NetPacket_C2S_ModifyTilemap * packet)
{
	DBG_LOG("Performing tilemap action %i.", packet->Action);

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);

	// Create the Undo operation.
	EditorTilemapModifyAction* action = new EditorTilemapModifyAction(packet);
	action->Do();

	Push_Undo(action);
}

void EditorManager::Perform_Action(NetPacket_C2S_EditorRepositionObjects * packet)
{
	DBG_LOG("Performing resposition action for %i objects.", packet->States.size());

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);

	// Create the Undo operation.
	EditorObjectRepositionAction* action = new EditorObjectRepositionAction(packet);
	action->Do();

	Push_Undo(action);
}

void EditorManager::Perform_Action(NetPacket_C2S_EditorDeleteObjects * packet)
{
	DBG_LOG("Performing deletion of %i objects.", packet->IDs.size());

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);

	// Create the Undo operation.
	EditorObjectDeletionAction* action = new EditorObjectDeletionAction(packet);
	action->Do();

	Push_Undo(action);
}

void EditorManager::Perform_Action(NetPacket_C2S_EditorCreateObjects * packet)
{
	DBG_LOG("Performing creation of %i objects.", packet->States.size());

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);

	// Create the Undo operation.
	EditorObjectCreationAction* action = new EditorObjectCreationAction(packet);
	action->Do();

	Push_Undo(action);
}

void EditorManager::Perform_Action(NetPacket_C2S_EditorChangeObjectState * packet)
{
	DBG_LOG("Performing state modification of object %i.", packet->ID);

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);

	// Create the Undo operation.
	EditorChangeObjectStateAction* action = new EditorChangeObjectStateAction(packet);
	action->Do();

	Push_Undo(action);
}

void EditorManager::Perform_Action(NetPacket_C2S_EditorChangeMapState * packet)
{
	DBG_LOG("Performing state modification of map.");

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);

	// Create the Undo operation.
	EditorChangeMapStateAction* action = new EditorChangeMapStateAction(packet);
	action->Do();

	Push_Undo(action);
}

void EditorManager::Perform_Action(NetPacket_C2S_EditorResetMap * packet)
{
	DBG_LOG("Performing reset of map.");

	Map* map = Game::Get()->Get_Map();
	Atlas* tileset = map->Get_Tileset()->Get();
	AtlasFrame* frame = tileset->Get_Frame_By_Index(0);
	Scene* scene = Game::Get()->Get_Scene();

	// Unload current map.
	GameEngine::Get()->Restart_Map();
	ActorReplicator::Get()->Restart(true);
	ParticleManager::Get()->Reset();
	CollisionManager::Get()->Reset();
	GameEngine::Get()->Get_Scene()->Unload();

	// Clear the map.
	MapFileHeaderBlock header;
	header.Author = "< not set >";
	header.Description = "< not set >";
	header.Height = DEFAULT_MAP_HEIGHT;
	header.Width = DEFAULT_MAP_WIDTH;
	header.Loading_Slogan = "< not set >";
	header.Long_Name = "< not set >";
	header.Max_Players = 4;
	header.Min_Players = 1;
	header.Music_Path = "music_bommirific";
	header.Music = ResourceFactory::Get()->Get_Sound(header.Music_Path.c_str());
	header.Short_Name = "< not set >";
	//header.Preview_Image = NULL;
	header.Preview_Path = "";
	header.Tileset_Name = "Main";
	header.Tileset = ResourceFactory::Get()->Get_Atlas(header.Tileset_Name.c_str());

	map->Set_Map_Header(header);
	map->Reset_Map_IDs();
	map->Resize(header.Width, header.Height, DEFAULT_MAP_DEPTH);
	map->Set_Tileset(header.Tileset);
	scene->Set_Boundries(Rect2D(0.0f, 0.0f, (float)map->Get_Width() * map->Get_Tile_Width(), (float)map->Get_Height() * map->Get_Tile_Height()));
	map->Apply_Header_Changes();

	map->Mark_Dirty();
	map->Mark_Collision_Dirty();

	// Clear all undo stacks as they are no longer valid.
	Clear_Undo_Stacks();

	// Update editor.
	UIScene_Editor* editor = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Editor*>();
	if (editor != NULL)
	{
		editor->Map_Properties_Changed();
		editor->Objects_Deleted();
	}
}

void EditorManager::Replicate_Client_Action(NetPacket * packet)
{
	DBG_LOG("Requesting client map action action '%s'.", packet->Get_Name());

	NetManager::Get()->Client()->Get_Connection()->Send(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}

void EditorManager::Replicate_Client_Undo()
{
	DBG_LOG("Requesting client undo.");
	
	NetPacket_C2S_EditorRequestUndo packet;
	NetManager::Get()->Client()->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}

void EditorManager::Replicate_Client_Redo()
{
	DBG_LOG("Requesting client redo.");
	
	NetPacket_C2S_EditorRequestRedo packet;
	NetManager::Get()->Client()->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}

void EditorManager::Trim_Undo_Stacks()
{
	while (m_undo_stack.size() > max_undo_stack_size)
	{
		std::vector<EditorAction*>::iterator iter = m_undo_stack.begin();
		SAFE_DELETE(*iter);
		m_undo_stack.erase(iter);
	}
	while (m_redo_stack.size() > max_undo_stack_size)
	{
		std::vector<EditorAction*>::iterator iter = m_redo_stack.begin();
		SAFE_DELETE(*iter);
		m_redo_stack.erase(iter);
	}
}

void EditorManager::Clear_Undo_Stacks()
{
	while (m_undo_stack.size() > 0)
	{
		std::vector<EditorAction*>::iterator iter = m_undo_stack.begin();
		SAFE_DELETE(*iter);
		m_undo_stack.erase(iter);
	}
	while (m_redo_stack.size() > 0)
	{
		std::vector<EditorAction*>::iterator iter = m_redo_stack.begin();
		SAFE_DELETE(*iter);
		m_redo_stack.erase(iter);
	}
}

void EditorManager::Push_Undo(EditorAction* action)
{
	m_undo_stack.push_back(action);

	// Clear redo stack.
	for (std::vector<EditorAction*>::iterator iter = m_redo_stack.begin(); iter != m_redo_stack.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_redo_stack.clear();

	Trim_Undo_Stacks();
}

void EditorManager::Undo()
{
	if (m_undo_stack.size() > 0)
	{
		EditorAction* action = m_undo_stack.back();
		m_undo_stack.pop_back();

		action->Undo();

		m_redo_stack.push_back(action);
	}

	Trim_Undo_Stacks();
}

void EditorManager::Redo()
{
	if (m_redo_stack.size() > 0)
	{
		EditorAction* action = m_redo_stack.back();
		m_redo_stack.pop_back();

		action->Do();

		m_undo_stack.push_back(action);
	}

	Trim_Undo_Stacks();
}

ScriptedActor* EditorManager::Get_Actor_From_Editor_ID(s32 id)
{
	// Replicated.
	if (id >= 0)
	{
		return ActorReplicator::Get()->Get_Actor_By_ID(id);
	}

	// Normal object.
	else 
	{
		return Game::Get()->Get_Game_Scene()->Get_Actor_By_Map_ID(-id);
	}
}

s32 EditorManager::Get_Editor_ID(ScriptedActor* actor)
{
	ActorReplicationInfo& rep_info = actor->Get_Replication_Info();
	
	// Replicated.
	if (actor->Get_Script_Object().Get()->Get_Symbol()->symbol->class_data->is_replicated)
	{
		return rep_info.unique_id;
	}

	// Non-replicated.
	else
	{
		return -(int)actor->Get_Map_ID();
	}
}

void EditorManager::Transfer_And_Open_Map(std::string path)
{
	NetManager* manager = NetManager::Get();
	NetServer* server = manager->Server();
	NetClient* client = manager->Client();
	
	// Client.
	if (client != NULL)
	{
		DBG_LOG("Transfering '%s' to server.", path.c_str());

		// Transfer to server.
		NetFileTransfer* transfer = new NetFileTransfer(INTERNAL_EDITOR_MAP_GUID, client->Get_Connection(), false);
		transfer->Send(path);
	}

	// Server
	else
	{
		DBG_ASSERT_STR(false, "Transfer_And_Open_Map should not be called on a server!");
	}
}

bool EditorManager::Callback_Recieve_Server_Open_Map(NetPacket_C2S_EditorOpenMap * packet)
{
	if (!All_Users_In_Game())
	{
		DBG_LOG("Ignoring action because all users are not in game.");
		return false;
	}

	if (GameNetManager::Get()->Game_Server()->Get_Server_State().In_Editor_Mode == false)
	{
		DBG_LOG("User '%s' attempted to perform editor open map whilst not in editor mode!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Check we are not already opening a map.
	if (m_map_open_queued == true)
	{
		DBG_LOG("User '%s' attempted to perform editor open map - map opening already in progress!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	// Check we have file in question.
	NetFileTransfer* transfer = packet->Get_Recieved_From()->Get_File_Transfer(packet->slot_hash, true);
	if (transfer == NULL)
	{
		DBG_LOG("User '%s' attempted to perform editor open map - but file could not be found!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}
	if (!transfer->Is_Finished())
	{
		DBG_LOG("User '%s' attempted to perform editor open map - but file transfer was not complete!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());
		return true;
	}

	DBG_LOG("User '%s' performing editor open map!", packet->Get_Recieved_From()->Get_Online_User()->Get_Username().c_str());

	// Queue up map open.
	m_map_open_queued = true;
	m_map_open_slot_hash = packet->slot_hash;
	m_map_open_buffer = transfer->Get_Data();
	m_map_open_enter_editor = false;

	// Broadcast file to all clients.
	// Note: Make sure to do this after copying transfer data as we will overwrite buffer when we
	//		 send data back to the initiating user.
	std::vector<GameNetUser*> users = GameNetManager::Get()->Get_Game_Net_Users();
	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		if (user->Get_Local_User_Index() == 0) // Only need to send it to one local user.
		{
			NetFileTransfer* subtransfer = new NetFileTransfer(packet->slot_hash, user->Get_Connection(), false);
			subtransfer->Send(m_map_open_buffer);
		}
	}

	return true;
}

bool EditorManager::Callback_Recieve_Client_Open_Map(NetPacket_C2S_EditorOpenMap * packet)
{
	// Clients always accept servers request to change maps, unless they are local-hosting,
	// in which case the modification has already been performed.
	if (NetManager::Get()->Server() == NULL)
	{
		// Grab map transfer.
		NetFileTransfer* transfer = packet->Get_Recieved_From()->Get_File_Transfer(packet->slot_hash, true);
		DBG_ASSERT_STR(transfer != NULL, "Could not find map transfer for opening editor map.");
		DBG_ASSERT_STR(transfer->Is_Finished(), "Editor map transfer was not finished! Should be finished before this is called.");

		// Update the editor map file.
		Update_Editor_Map_File(transfer->Get_Data());

		// The server will now force a map change which will make us load our now updated editor map file!
	}

	return true;
}

void EditorManager::Update_Editor_Map_File(DataBuffer buffer)
{
	DBG_LOG("Updating editor map file ...");

	// Load the map data into a map file handle that can be loaded.
	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(INTERNAL_EDITOR_MAP_GUID);
	if (handle == NULL)
	{
		handle = ResourceFactory::Get()->New_Map(INTERNAL_EDITOR_MAP_GUID);
	}

	// Load new map data.
	BinaryStream stream(buffer.Buffer(), buffer.Size());
	if (!handle->Get()->Load_Internal(&stream))
	{
		DBG_LOG("Failed to load editor map file! This shouldn't happen - possible problems or desyncs due to this!");
	}

	// Patch header data so when retrieving maps we get the correct one.
	handle->Get()->Set_Internal(true);
	handle->Get()->Get_Header()->GUID = INTERNAL_EDITOR_MAP_GUID;
}


void EditorManager::Tick(const FrameTime& time)
{
	NetManager* manager = NetManager::Get();
	NetServer* server = manager->Server();
	NetClient* client = manager->Client();

	if (client != NULL)
	{
		// Are we currently sending map?
		NetFileTransfer* transfer = client->Get_Connection()->Get_File_Transfer(INTERNAL_EDITOR_MAP_GUID, false);
		if (transfer != NULL)
		{
			if (transfer->Is_Finished())
			{
				DBG_LOG("Requesting server change map to map in slot '%s'.", INTERNAL_EDITOR_MAP_GUID);
				
				NetPacket_C2S_EditorOpenMap packet;
				packet.slot_hash = transfer->Get_Slot_Hash();
				client->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_FILE_TRANSFER, true);

				SAFE_DELETE(transfer);
			}
		}
	}

	if (server != NULL)
	{
		// Is a map open queued?
		if (m_map_open_queued == true)
		{
			// Have we finished sending map to all users?
			bool transfers_in_progress = false;
			
			std::vector<GameNetUser*> users = GameNetManager::Get()->Get_Game_Net_Users();
			for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
			{
				GameNetUser* user = *iter;
				NetFileTransfer* transfer = user->Get_Connection()->Get_File_Transfer(m_map_open_slot_hash, false);
				if (transfer != NULL)
				{
					if (!transfer->Is_Finished())
					{
						transfers_in_progress = true;
						break;
					}
				}
			}

			// Time to start the open!
			if (transfers_in_progress == false)
			{
				// Update our editor map file with the new data.
				Update_Editor_Map_File(m_map_open_buffer);

				// Tell all users to update their map files.
				for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
				{
					GameNetUser* user = *iter;

					NetPacket_C2S_EditorOpenMap packet;
					packet.slot_hash = m_map_open_slot_hash;
					user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
				}

				// Force map change to our now updated map file!
				NetManager::Get()->Server()->Force_Map_Change(
					INTERNAL_EDITOR_MAP_GUID,
					0,
					Random::Static_Next(),
					0);
			
				// Enter editor mode?
				if (m_map_open_enter_editor == true)
				{
					GameNetManager::Get()->Game_Server()->Enter_Editor_Mode();
					m_map_open_enter_editor = false;
				}

				m_map_open_queued = false;
			}
		}
	}
}
