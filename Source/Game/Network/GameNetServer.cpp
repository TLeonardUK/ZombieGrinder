// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetUser.h"

#include "Game/Network/ActorReplicator.h"

#include "Engine/Localise/Locale.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"

#include "Game/Game/ChatManager.h"
#include "Game/Game/VoteManager.h"
#include "Game/Game/EditorManager.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineServer.h"

#include "Game/Profile/ProfileManager.h"

#include "Game/Runner/GameOptions.h"

GameNetServer::GameNetServer()
	: m_callback_game_state_changed_recieved(this, &GameNetServer::Callback_GameStateChangedRecieved, false, NULL)
	, m_callback_profile_update_recieved(this, &GameNetServer::Callback_ProfileUpdateRecieved, false, NULL)
	, m_callback_client_state_update_recieved(this, &GameNetServer::Callback_ClientStateUpdateRecieved, false, NULL)
	, m_callback_team_change_request_recieved(this, &GameNetServer::Callback_TeamChangeRequestRecieved, false, NULL)
	, m_user_state_broadcast_timer(0.0f)
	, m_deferred_editor_toggle(false)
	, m_enter_editor_on_load(false)
{
	m_server_state.Name = "";
	m_server_state.Host_Name = "";
	m_server_state.Elapsed_Map_Time = 0.0f;
	m_server_state.In_Editor_Mode = false;
	m_server_state.Game_Mode_Net_ID = 0;
}

bool GameNetServer::Init()
{
	if (!NetServer::Init())
	{
		return false;
	}

	return true;
}

void GameNetServer::Tick(const FrameTime& time)
{
	NetServer::Tick(time);
	
	Update_Server_State(time);
	Update_User_States(time);

	// Prevent users joining during editor mode.
	Set_Joining_Enabled(!m_server_state.In_Editor_Mode);

	// Check deferred editor mode.
	if (m_deferred_editor_toggle)
	{
		bool all_in_game = true;

		std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();
		for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
		{
			GameNetUser* user = static_cast<GameNetUser*>(*iter);
			if (user->Get_Game_State() != GameNetClient_GameState::InGame &&
				user->Get_Game_State() != GameNetClient_GameState::InCutscene)
			{		
				all_in_game = false;
			}
		}

		if (all_in_game && net_users.size() > 0)
		{
			Toggle_Editor_Mode();
			m_deferred_editor_toggle = false;
		}
	}
}

void GameNetServer::Update_User_States(const FrameTime& time)
{
	m_user_state_broadcast_timer += time.Get_Frame_Time();

	if (m_user_state_broadcast_timer > *GameOptions::network_user_state_update_rate)
	{
		std::vector<GameNetUser*> net_users = GameNetManager::Get()->Get_Game_Net_Users();

		NetPacket_S2C_UserStateUpdate packet;
		
		for (std::vector<GameNetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
		{
			GameNetUser* user = *iter;
			GameNetUserState* state = user->Get_State();

			UserPacketState user_state;
			user_state.User_ID = user->Get_Net_ID();
			user_state.Game_State = (u8)user->Get_Game_State();

			state->Serialize(&user_state);

			packet.States.push_back(user_state);
		}
		
		Get_Connection()->Broadcast(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

		m_user_state_broadcast_timer = 0.0f;
	}
}

void GameNetServer::Update_Server_State(const FrameTime& time)
{
	bool major_change = false;

	m_server_state.Elapsed_Map_Time += time.Get_Frame_Time();

	std::string host_name = OnlinePlatform::Get()->Server()->Get_Host_Name();
	if (m_server_state.Host_Name != host_name)
	{
		m_server_state.Host_Name = host_name;
		major_change = true;
	}
	
	std::string server_name = OnlinePlatform::Get()->Server()->Get_Server_Name();
	if (m_server_state.Name != server_name)
	{
		m_server_state.Name	= server_name;
		major_change = true;
	}

	int net_id = Game::Get()->Get_Game_Mode()->Get_Active_Net_ID();
	if (m_server_state.Game_Mode_Net_ID != net_id)
	{
		m_server_state.Game_Mode_Net_ID = net_id;
		major_change = true;
	}

	// If any of the major settings have changed, lets sync to the client.
	if (major_change == true)
	{
		Broadcast_Server_State();
	}
}

void GameNetServer::Broadcast_Server_State()
{
	std::vector<GameNetUser*> users = GameNetManager::Get()->Get_Game_Net_Users();
	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;

		// We only need to send it to one local user!
		if (user->Get_Local_User_Index() == 0)
		{
			Send_Server_State(user);
		}
	}
}

void GameNetServer::SendPreInitialMapChangeInfo(NetUser* user)
{
	Send_Server_State(static_cast<GameNetUser*>(user));
}

void GameNetServer::Send_Server_State(GameNetUser* user)
{
	NetPacket_S2C_ServerStateChanged packet;
	packet.State = m_server_state;

	user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}

void GameNetServer::Synchronize_User(GameNetUser* user)
{
	// If user is localhosting, no need to sync.
	if (user->Get_Online_User()->Is_Local())
	{
		return;
	}

	DBG_LOG("Sending server sync to user '%s'", user->Get_Username().c_str());

	// Sync server state.
	Send_Server_State(user);

	// Send all other user profiles.
	std::vector<NetUser*> all_users = NetManager::Get()->Get_Net_Users();
	for (std::vector<NetUser*>::iterator iter = all_users.begin(); iter != all_users.end(); iter++)
	{
		GameNetUser* other_user = static_cast<GameNetUser*>(*iter);
		Profile* profile = other_user->Get_Profile();
		if (profile != NULL)
		{
			NetPacket_C2S_ProfileUpdate packet;
			packet.User_ID		= other_user->Get_Net_ID(); // Seems to fuck off, packet sending issue?>
			packet.Profile_Data = ProfileManager::Get()->Profile_To_Buffer(profile);
			user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

			DBG_LOG("Syncing profile for user %u to new user (size %i).", packet.User_ID, packet.Profile_Data.Size());
		}
	}
	
	// Send all actors.
	ActorReplicator::Get()->Send_Full_Update(user);

	// Send possession changes for each actor.		
	std::vector<GameNetUser*> list = GameNetManager::Get()->Get_Game_Net_Users();
	for (std::vector<GameNetUser*>::iterator iter = list.begin(); iter != list.end(); iter++)
	{
		GameNetUser* other = *iter;
		if (other != user)
		{
			ScriptedActor* other_actor = other->Get_Controller();
			if (other_actor != NULL)
			{
				NetPacket_S2C_UserPossessionChanged packet;
				packet.User_ID	= other->Get_Net_ID();
				packet.Actor_ID = other_actor->Get_Replication_Info().unique_id;

				user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
			}
		}
	}

	// Broadcast current vote status.
	if (VoteManager::Get()->Get_Active_Vote() != NULL)
	{
		VoteManager::Get()->Broadcast_Update();
	}
}

bool GameNetServer::Callback_TeamChangeRequestRecieved(NetPacket_C2S_RequestTeamChange* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	NetConnection* connection = packet->Get_Recieved_From();

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (user->Get_Connection() == connection && user->Get_Net_ID() == packet->User_ID)
		{			
			DBG_LOG("Recieved team change request (to team %i) for user '%s' (net id = %u).", packet->Team_Index, user->Get_Username().c_str(),(u32)user->Get_Net_ID());

			GameMode* mode = Game::Get()->Get_Game_Mode();
			if (mode != NULL)
			{
				std::vector<GameModeTeam> teams = mode->Get_Teams();
				if (packet->Team_Index >= 0 && packet->Team_Index < (int)teams.size())
				{
					GameModeTeam& team = teams.at(packet->Team_Index);

					// Tell everyone user changed team.
					ChatManager::Get()->Send_Server(ChatMessageType::Server, SF("#chat_message_player_team_change", user->Get_Display_Username().c_str(), team.Primary_Color.R, team.Primary_Color.G, team.Primary_Color.B, S(team.Name)));

					// Destroy all users actors.
					ActorReplicator::Get()->Despawn_Users_Actors(user);

					// Send notice to other users.
					Get_Connection()->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

					user->Get_State()->Team_Index = packet->Team_Index;
				}
			}
		}
	}

	return true;
}

bool GameNetServer::Callback_GameStateChangedRecieved(NetPacket_C2S_GameStateChanged* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	NetConnection* connection = packet->Get_Recieved_From();

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (user->Get_Connection() == connection)
		{			
			DBG_LOG("Recieved game state change (to %i) for user '%s' (net id = %u).", packet->New_State, user->Get_Username().c_str(),(u32)user->Get_Net_ID());
			user->Set_Map_Load_Index(packet->Map_Load_Index);
			user->Set_Game_State(packet->New_State, true);		
		}
	}

	return true;
}

bool GameNetServer::Callback_ClientStateUpdateRecieved(NetPacket_C2S_ClientUserStateUpdate* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	NetConnection* connection = packet->Get_Recieved_From();

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (user->Get_Connection() == connection &&
			packet->User_ID == user->Get_Net_ID())
		{	
			user->Recieve_State_Update(packet);
			break;
		}
	}

	return true;
}

bool GameNetServer::Callback_ProfileUpdateRecieved(NetPacket_C2S_ProfileUpdate* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	NetConnection* connection = packet->Get_Recieved_From();

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (user->Get_Connection() == connection &&
			packet->User_ID == user->Get_Net_ID())
		{			
			DBG_LOG("Recieved profile update for user '%s' (net id = %u).",user->Get_Username().c_str(),(u32)user->Get_Net_ID());
			
			user->Set_Pending_Profile(ProfileManager::Get()->Buffer_To_Profile(packet->Profile_Data));
			if (user->Get_Profile() == NULL)
			{
				user->Apply_Pending_Profile();
			}

			break;
		}
	}

	return true;
}

void GameNetServer::Accept_Pending_Profile_Changes()
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		user->Apply_Pending_Profile();
	}
}

void GameNetServer::On_Connected(NetConnection* connection, NetPacket_C2S_Connect* packet)
{
	// Force update of editor map file incase user needs to download it.
	if (m_server_state.In_Editor_Mode == true)
	{
		GameNetManager::Get()->Editor_Mode_Changed();
	}

	NetServer::On_Connected(connection, packet);
}

void GameNetServer::On_User_Joined(NetUser* user)
{
	ChatManager::Get()->Send_Server(ChatMessageType::Server, SF("#chat_message_player_join", user->Get_Display_Username().c_str()));
}

void GameNetServer::On_User_Left(NetUser* user)
{
	ChatManager::Get()->Send_Server(ChatMessageType::Server, SF("#chat_message_player_leave", user->Get_Display_Username().c_str()));

	// Destroy all actors belonging to this user.
	static_cast<GameNetUser*>(user)->Restart();
	static_cast<GameNetUser*>(user)->LeftGame();
	ActorReplicator::Get()->Despawn_Users_Actors(static_cast<GameNetUser*>(user));
}

void GameNetServer::On_User_Name_Change(NetUser* user, std::string old_username, std::string new_username)
{
	ChatManager::Get()->Send_Server(ChatMessageType::Server, SF("#chat_message_player_rename", old_username.c_str(), new_username.c_str()));
}

void GameNetServer::Map_Loaded()
{
	m_server_state.Elapsed_Map_Time = 0.0f;

	if (m_enter_editor_on_load)
	{
		m_server_state.In_Editor_Mode = true;
		GameNetManager::Get()->Editor_Mode_Changed();
		Broadcast_Server_State();

		m_enter_editor_on_load = false;
	}
}

void GameNetServer::Toggle_Editor_Mode()
{
	bool all_in_game = true;

	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();
	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (user->Get_Game_State() != GameNetClient_GameState::InGame &&
			user->Get_Game_State() != GameNetClient_GameState::InCutscene)
		{		
			all_in_game = false;
		}
	}

	if (all_in_game == false || net_users.size() == 0)
	{
		m_deferred_editor_toggle = true;
		return;
	}

	// If we are entering editor mode - reset to last "safe" state.
	if (!m_server_state.In_Editor_Mode)
	{
		// Restart map and editor editor mode.
		NetManager::Get()->Server()->Restart_Map();
		m_enter_editor_on_load = true;

		Broadcast_Server_State();

		//EditorManager::Get()->Reload_Map_State();		
	}

	// Exiting editor mode - save last state and go for it.
	else
	{
		//EditorManager::Get()->Store_Map_State();

		Game::Get()->Finalize_Map_Load();
		ScriptEventListener::Fire_On_Map_Load();
		GameNetManager::Get()->Game_Server()->Map_Loaded();

		m_server_state.In_Editor_Mode = false;
		GameNetManager::Get()->Editor_Mode_Changed();
		Broadcast_Server_State();
	}
}

void GameNetServer::Reset_Settings()
{
	m_server_state.Elapsed_Map_Time = 0.0f;
	m_server_state.In_Editor_Mode = false;

	NetServer::Reset_Settings();
}

void GameNetServer::Enter_Editor_Mode()
{
	m_server_state.In_Editor_Mode = true;
	GameNetManager::Get()->Editor_Mode_Changed();
	Broadcast_Server_State();
}

GameNetServerState& GameNetServer::Get_Server_State()
{
	return m_server_state;
}
