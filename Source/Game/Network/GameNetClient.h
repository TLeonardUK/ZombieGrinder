// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_GAME_NET_CLIENT_
#define _GAME_NETWORK_GAME_NET_CLIENT_

#include "Engine/Network/NetClient.h"
#include "Game/Network/GameNetClient_GameState.h"

#include "Game/Network/Packets/GamePackets.h"

class GameNetClient : public NetClient
{
	MEMORY_ALLOCATOR(GameNetClient, "Network");

private:
	GameNetClient_GameState::Type m_game_state;

	GameNetServerState m_server_state;

	bool m_last_editor_mode;

	float m_time_till_next_clinet_state;

	float m_start_time;

	enum
	{
		client_state_send_interval = 300
	};

private:
	PACKET_RECIEVE_CALLBACK(GameNetClient, Callback_ServerStateChangedRecieved, NetPacket_S2C_ServerStateChanged,  m_callback_server_state_changed_recieved);
	PACKET_RECIEVE_CALLBACK(GameNetClient, Callback_UserStateUpdateRecieved, NetPacket_S2C_UserStateUpdate,  m_callback_user_state_update_recieved);
	PACKET_RECIEVE_CALLBACK(GameNetClient, Callback_UserPossessionChanged, NetPacket_S2C_UserPossessionChanged,  m_callback_user_possession_changed);
	PACKET_RECIEVE_CLIENT_CALLBACK(GameNetClient, Callback_ProfileUpdateRecieved, NetPacket_C2S_ProfileUpdate,  m_callback_profile_update_recieved);
	PACKET_RECIEVE_CLIENT_CALLBACK(GameNetClient, Callback_TeamChangeRequestRecieved, NetPacket_C2S_RequestTeamChange,  m_callback_team_change_request_recieved);

public:
	GameNetClient();

	bool Init();

	void Set_Game_State(GameNetClient_GameState::Type state);
	GameNetClient_GameState::Type Get_Game_State();

	void Send_Profile_Update();

	GameNetServerState& Get_Server_State();
	
	void On_Connected(NetConnection* connection);
	void On_Disconnected(NetConnection* connection);

	void Request_Team_Change(GameNetUser* user, int team_index);

	void Tick(const FrameTime& time);
	void Predict_Server_State(const FrameTime& time);

	void On_User_Left(NetUser* user);
};

#endif

