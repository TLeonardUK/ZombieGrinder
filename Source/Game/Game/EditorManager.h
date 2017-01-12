// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_EDITOR_MANAGER_
#define _GAME_EDITOR_MANAGER_

#include "Engine/Network/NetClient.h"

#include "Game/Network/Packets/GamePackets.h"

class EditorAction;
class ScriptedActor;

// Used to name and reference an internal editor map used for opening while editing.
#define INTERNAL_EDITOR_MAP_GUID "InEditorInternalMap"

class EditorManager : public Singleton<EditorManager>
{
	MEMORY_ALLOCATOR(EditorManager, "Game");

private:

private:
	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Tilemap_Modification, NetPacket_C2S_ModifyTilemap, m_callback_server_tilemap_modification_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Tilemap_Modification, NetPacket_C2S_ModifyTilemap, m_callback_client_tilemap_modification_recieved);
	
	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Object_Reposition, NetPacket_C2S_EditorRepositionObjects, m_callback_server_object_reposition_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Object_Reposition, NetPacket_C2S_EditorRepositionObjects, m_callback_client_object_reposition_recieved);
	
	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Delete_Objects, NetPacket_C2S_EditorDeleteObjects, m_callback_server_delete_objects_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Delete_Objects, NetPacket_C2S_EditorDeleteObjects, m_callback_client_delete_objects_recieved);
	
	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Create_Objects, NetPacket_C2S_EditorCreateObjects, m_callback_server_create_objects_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Create_Objects, NetPacket_C2S_EditorCreateObjects, m_callback_client_create_objects_recieved);
	
	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Change_Object, NetPacket_C2S_EditorChangeObjectState, m_callback_server_change_object_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Change_Object, NetPacket_C2S_EditorChangeObjectState, m_callback_client_change_object_recieved);
	
	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Change_Map, NetPacket_C2S_EditorChangeMapState, m_callback_server_change_map_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Change_Map, NetPacket_C2S_EditorChangeMapState, m_callback_client_change_map_recieved);

	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Reset_Map, NetPacket_C2S_EditorResetMap, m_callback_server_reset_map_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Reset_Map, NetPacket_C2S_EditorResetMap, m_callback_client_reset_map_recieved);

	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Open_Map, NetPacket_C2S_EditorOpenMap, m_callback_server_open_map_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Open_Map, NetPacket_C2S_EditorOpenMap, m_callback_client_open_map_recieved);

	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Undo_Request, NetPacket_C2S_EditorRequestUndo, m_callback_server_undo_request_recieved);
	PACKET_RECIEVE_SERVER_CALLBACK(EditorManager, Callback_Recieve_Server_Redo_Request, NetPacket_C2S_EditorRequestRedo, m_callback_server_redo_request_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Undo_Request, NetPacket_C2S_EditorRequestUndo, m_callback_client_undo_request_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(EditorManager, Callback_Recieve_Client_Redo_Request, NetPacket_C2S_EditorRequestRedo, m_callback_client_redo_request_recieved);

	std::vector<EditorAction*> m_undo_stack;
	std::vector<EditorAction*> m_redo_stack;

	int m_editor_action_index;

	bool m_map_open_queued;
	int m_map_open_slot_hash;
	DataBuffer m_map_open_buffer;

	DataBuffer m_stored_map_data;
	bool m_map_open_enter_editor;

	enum
	{
		max_undo_stack_size = 8,
	};

protected:
	void Perform_Action(NetPacket_C2S_ModifyTilemap * packet);
	void Perform_Action(NetPacket_C2S_EditorRepositionObjects * packet);
	void Perform_Action(NetPacket_C2S_EditorDeleteObjects * packet);
	void Perform_Action(NetPacket_C2S_EditorCreateObjects * packet);
	void Perform_Action(NetPacket_C2S_EditorChangeObjectState * packet);
	void Perform_Action(NetPacket_C2S_EditorChangeMapState * packet);
	void Perform_Action(NetPacket_C2S_EditorResetMap * packet);

	void Push_Undo(EditorAction* action);
	void Undo();
	void Redo();

	bool All_Users_In_Game();

	void Clear_Undo_Stacks();
	void Trim_Undo_Stacks();

public:
	EditorManager();
	~EditorManager();

	void Update_Editor_Map_File(DataBuffer buffer);

	int Get_Editor_Action_Index();

	int Get_Undo_Stack_Size();
	int Get_Redo_Stack_Size();

	ScriptedActor* Get_Actor_From_Editor_ID(s32 id);
	s32 Get_Editor_ID(ScriptedActor* actor);

	void Transfer_And_Open_Map(std::string path);

	/*
	void Store_Map_State();
	void Reload_Map_State();
	*/

	void Replicate_Client_Action(NetPacket * packet);
	void Replicate_Client_Undo();
	void Replicate_Client_Redo();

	void Tick(const FrameTime& time);

};

#endif

