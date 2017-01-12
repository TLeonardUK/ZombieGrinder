// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Engine/EngineOptions.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/Runner/GameMode.h"

#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Runner/Game.h"
#include "Game/Scene/GameScene.h"

#include "Engine/Online/OnlineUser.h"
#include "Game/Profile/ProfileManager.h"

#include "Game/Network/ActorReplicator.h"

GameNetClient::GameNetClient()
	: m_game_state(GameNetClient_GameState::Idle)
	, m_last_editor_mode(false)
	, m_time_till_next_clinet_state(0.0f)
	, m_callback_server_state_changed_recieved(this, &GameNetClient::Callback_ServerStateChangedRecieved, false, NULL)
	, m_callback_user_state_update_recieved(this, &GameNetClient::Callback_UserStateUpdateRecieved, false, NULL)
	, m_callback_user_possession_changed(this, &GameNetClient::Callback_UserPossessionChanged, false, NULL)
	, m_callback_profile_update_recieved(this, &GameNetClient::Callback_ProfileUpdateRecieved, false, NULL)
	, m_callback_team_change_request_recieved(this, &GameNetClient::Callback_TeamChangeRequestRecieved, false, NULL)
	, m_start_time(0.0f)
{
}

bool GameNetClient::Init()
{
	if (!NetClient::Init())
	{
		return false;
	}

	return true;
}

void GameNetClient::Tick(const FrameTime& time)
{
	NetClient::Tick(time);

	Predict_Server_State(time);

#ifndef MASTER_BUILD

	// Close down client if above lifetime.
	double ticks = Platform::Get()->Get_Ticks();

	if (*EngineOptions::client_lifetime != 0)
	{
		if (ticks - m_start_time > *EngineOptions::client_lifetime)
		{
			GameEngine::Get()->Stop();
		}
	}

#endif

	// Periodically sent a client update.
	m_time_till_next_clinet_state -= time.Get_Frame_Time();
	if (m_time_till_next_clinet_state < 0)
	{
		Camera* camera = Game::Get()->Get_Camera(CameraID::Game1);
		if (camera != NULL)
		{
			std::vector<NetUser*> users = NetManager::Get()->Get_Local_Net_Users();
			int index = 0;
			for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++, index++)
			{
				GameNetUser* user = static_cast<GameNetUser*>(*iter);

				NetPacket_C2S_ClientUserStateUpdate packet;
				packet.User_ID = user->Get_Net_ID();

				if (Game::Get()->Get_Game_Mode()->Is_PVP())
				{
					packet.Viewport = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + index))->Get_Bounding_Viewport();
				}
				else
				{
					packet.Viewport = camera->Get_Bounding_Viewport();
				}

				packet.Level = user->Get_Local_Profile()->Level;

				// We don't give a dam about this being reliable.
				Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_STATE_UNRELIABLE, false);	
			}

			m_time_till_next_clinet_state = client_state_send_interval;
		}
	}
}

void GameNetClient::Predict_Server_State(const FrameTime& time)
{
	// If local-hosting, just grab server state.
	GameNetServer* server = GameNetManager::Get()->Game_Server();
	if (server != NULL)
	{
		m_server_state = server->Get_Server_State();
	}
	else
	{	
		// We increment the server time ourselves, waste of bandwidth syncing it.
		m_server_state.Elapsed_Map_Time += time.Get_Frame_Time();
	}
}	

void GameNetClient::Send_Profile_Update()
{
	std::vector<NetUser*> users = NetManager::Get()->Get_Local_Net_Users();
	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		
		Profile* profile = ProfileManager::Get()->Get_Profile(user->Get_Online_User()->Get_Profile_Index());

		NetPacket_C2S_ProfileUpdate packet;
		packet.User_ID		= user->Get_Net_ID();
		packet.Profile_Data = ProfileManager::Get()->Profile_To_Buffer(profile);
		Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

		DBG_LOG("Sending profile update for local user %i.", user->Get_Net_ID());
	}
}

void GameNetClient::On_Connected(NetConnection* connection)
{
	NetClient::On_Connected(connection);
}

void GameNetClient::On_Disconnected(NetConnection* connection)
{	
	NetClient::On_Disconnected(connection);

	Set_Game_State(GameNetClient_GameState::Idle);
}

void GameNetClient::Set_Game_State(GameNetClient_GameState::Type state)
{
	if (Get_State() == NetClient_State::Connected)
	{
		// Update the state of all local users.
		std::vector<NetUser*> users = NetManager::Get()->Get_Net_Users();
		for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			GameNetUser* user = static_cast<GameNetUser*>(*iter);
			if (user->Get_Online_User()->Is_Local())
			{
				user->Set_Game_State(state);
			}
		}

		NetPacket_C2S_GameStateChanged packet;
		packet.New_State = state;
		packet.Map_Load_Index = NetManager::Get()->Get_Current_Map_Load_Index();
		Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
	}

	DBG_LOG("Game client transitioned into game state %i.", state);
	m_game_state = state;
}

GameNetClient_GameState::Type GameNetClient::Get_Game_State()
{
	return m_game_state;
}
	
GameNetServerState& GameNetClient::Get_Server_State()
{
	return m_server_state;
}

void GameNetClient::Request_Team_Change(GameNetUser* user, int team_index)
{
	DBG_LOG("User '%s' requested to change team to %i.", user->Get_Username().c_str(), team_index);

	NetPacket_C2S_RequestTeamChange packet;
	packet.User_ID = user->Get_Net_ID();
	packet.Team_Index = team_index;
	Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}

bool GameNetClient::Callback_TeamChangeRequestRecieved(NetPacket_C2S_RequestTeamChange* packet)
{
	GameNetUser* user = GameNetManager::Get()->Get_Game_User_By_Net_ID(packet->User_ID);
	if (user != NULL)
	{
		GameMode* mode = Game::Get()->Get_Game_Mode();
		if (mode != NULL)
		{
			std::vector<GameModeTeam> teams = mode->Get_Teams();
			if (packet->Team_Index >= 0 && packet->Team_Index < (int)teams.size())
			{
				DBG_LOG("Server has told us that for user '%s' (net id = %u) has changed to team %i.", user->Get_Username().c_str(), (u32)user->Get_Net_ID(), packet->Team_Index);

				// Destroy all users actors.
				ActorReplicator::Get()->Despawn_Users_Actors(user);

				user->Get_State()->Team_Index = packet->Team_Index;
			}
		}
	}

	return true;
}

bool GameNetClient::Callback_ServerStateChangedRecieved(NetPacket_S2C_ServerStateChanged* packet)
{
	DBG_LOG("Recieved new server state updated.");

	m_server_state = packet->State;

	if (m_server_state.In_Editor_Mode != m_last_editor_mode)
	{
		GameNetManager::Get()->Editor_Mode_Changed();
		m_last_editor_mode = m_server_state.In_Editor_Mode;
	}

	return true;
}

bool GameNetClient::Callback_UserStateUpdateRecieved(NetPacket_S2C_UserStateUpdate* packet)
{
	// We are authorative if the server, we don't want the user state!
	if (NetManager::Get()->Server() != NULL)
	{
		return true;
	}

	for (std::vector<UserPacketState>::iterator iter = packet->States.begin(); iter != packet->States.end(); iter++)
	{
		UserPacketState& state = *iter;

		GameNetUser* net_user = static_cast<GameNetUser*>(NetManager::Get()->Get_User_By_Net_ID(state.User_ID));
		if (net_user != NULL)
		{
			if (!net_user->Get_Online_User()->Is_Local())
			{
				net_user->Set_Game_State((GameNetClient_GameState::Type)state.Game_State, false);
			}
			net_user->Get_State()->Deserialize(&state);
		}
		else
		{
			DBG_LOG("Recieved state updated for unknown user-id '%i'. Recieved update before initial player list or after player disconnected?", state.User_ID);
		}
	}
	return true;
}

bool GameNetClient::Callback_UserPossessionChanged(NetPacket_S2C_UserPossessionChanged* packet)
{
	GameNetUser* user = GameNetManager::Get()->Get_Game_User_By_Net_ID(packet->User_ID);
	if (user != NULL)
	{
		if (GameNetManager::Get()->Game_Server() == NULL)
			user->Set_Controller_ID(packet->Actor_ID);
	}
	return true;
}

bool GameNetClient::Callback_ProfileUpdateRecieved(NetPacket_C2S_ProfileUpdate* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	DBG_LOG("Recieved profile update for net id %u, size of %i.", packet->User_ID, packet->Profile_Data.Size());

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (packet->User_ID == user->Get_Net_ID())
		{			
			DBG_LOG("Recieved profile update for user '%s' (net id = %u).",user->Get_Username().c_str(), packet->User_ID);
			user->Set_Profile(ProfileManager::Get()->Buffer_To_Profile(packet->Profile_Data));
			break;
		}
	}

	return true;
}

void GameNetClient::On_User_Left(NetUser* user)
{
	// Destroy all actors belonging to this user.
	static_cast<GameNetUser*>(user)->Restart();
	static_cast<GameNetUser*>(user)->LeftGame();
	ActorReplicator::Get()->Despawn_Users_Actors(static_cast<GameNetUser*>(user));
}