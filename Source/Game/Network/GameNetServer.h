// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_GAME_NET_SERVER_
#define _GAME_NETWORK_GAME_NET_SERVER_

#include "Engine/Network/NetServer.h"
#include "Game/Network/Packets/GamePackets.h"

class GameNetServer : public NetServer
{
	MEMORY_ALLOCATOR(GameNetServer, "Network");

private:
	PACKET_RECIEVE_CALLBACK(GameNetServer, Callback_GameStateChangedRecieved, NetPacket_C2S_GameStateChanged,  m_callback_game_state_changed_recieved);
	PACKET_RECIEVE_SERVER_CALLBACK(GameNetServer, Callback_ProfileUpdateRecieved, NetPacket_C2S_ProfileUpdate,  m_callback_profile_update_recieved);
	PACKET_RECIEVE_CALLBACK(GameNetServer, Callback_ClientStateUpdateRecieved, NetPacket_C2S_ClientUserStateUpdate,  m_callback_client_state_update_recieved);
	PACKET_RECIEVE_SERVER_CALLBACK(GameNetServer, Callback_TeamChangeRequestRecieved, NetPacket_C2S_RequestTeamChange,  m_callback_team_change_request_recieved);
	
	GameNetServerState m_server_state;

	float m_user_state_broadcast_timer;
		
	bool m_deferred_editor_toggle; 
	bool m_enter_editor_on_load;

public:
	GameNetServer();

	bool Init();

	void Tick(const FrameTime& time);

	void Update_Server_State(const FrameTime& time);
	void Send_Server_State(GameNetUser* user);
	void Broadcast_Server_State();

	void Reset_Settings();

	void SendPreInitialMapChangeInfo(NetUser* user);

	void Accept_Pending_Profile_Changes();

	GameNetServerState& Get_Server_State();

	void Map_Loaded();

	void Enter_Editor_Mode(); 
	void Toggle_Editor_Mode();

	void Update_User_States(const FrameTime& time);
	void Synchronize_User(GameNetUser* user);
	
	virtual void On_Connected(NetConnection* connection, NetPacket_C2S_Connect* packet);

	void On_User_Joined(NetUser* user);
	void On_User_Left(NetUser* user);
	void On_User_Name_Change(NetUser* user, std::string old_username, std::string new_username);

};

#endif

