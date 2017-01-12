// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"
#include "Game/Scene/GameScene.h"
#include "Game/Game/ChatManager.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/UI/Scenes/UIScene_MapLoading.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"

#include "Engine/Online/OnlineUser.h"
#include "Game/Profile/ProfileManager.h"

#include "Engine/UI/UIManager.h"

GameNetUser::~GameNetUser()
{
	SAFE_DELETE(m_event_listener);
	SAFE_DELETE(m_pending_profile);
	SAFE_DELETE(m_profile);
	SAFE_DELETE(m_state);
}

GameNetUser::GameNetUser(NetConnection* connection, int local_user_index, OnlineUser* online_user)
	: NetUser(connection, local_user_index, online_user)
	, m_game_state(GameNetClient_GameState::Idle)
	, m_is_synced(false)
	, m_map_load_index(-1)
	, m_full_update_pending(false)
	, m_script_object(NULL)
	, m_controller(NULL)
	, m_controller_id(-1)
	, m_base_tick(0.0f)
	, m_profile(NULL)
	, m_pending_profile(NULL)
	, m_profile_change_counter(0)
	, m_last_client_state_recieve_time(0.0f)
	, m_initial_level(0)
	, m_last_heading_position(0.0f, 0.0f, 0.0f)
	, m_heading(0.0f, 0.0f, 0.0f)
	, m_normalized_heading(0.0f, 0.0f, 0.0f)
	, m_heading_lerp_delta(0.3f)
	, m_scoreboard_rank(0)
{
	m_state = new GameNetUserState(this);

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* symbol = vm->Find_Class("NetUser");
	m_script_object = vm->New_Object(symbol, true, this);
	vm->Set_Default_State(m_script_object);

	if (online_user->Is_Local())
	{
		int index = online_user->Get_Profile_Index();
		DBG_LOG("'%s''s local profile index is %i.", online_user->Get_Username().c_str(), index);

		m_initial_level = ProfileManager::Get()->Get_Profile(index)->Level;
	}

	m_event_listener = new ScriptEventListener(vm->Get_Static_Context(), m_script_object);
}

CVMObjectHandle GameNetUser::Get_Script_Object()
{
	return m_script_object;
}

int GameNetUser::Get_Profile_Change_Counter()
{
	return m_profile_change_counter;
}

std::string GameNetUser::Get_Display_Username(std::string username)
{
	std::string old_username = m_username;
	m_username = username == "" ? m_username : username;
	std::string result = ChatManager::Get()->User_To_String(this);
	m_username = old_username;
	return result;
}

void GameNetUser::Update_Heading(ScriptedActor* actor)
{
	Vector3 new_heading = (actor->Get_Position() - m_last_heading_position).Normalize();
	m_heading = m_heading.Lerp(new_heading, GameEngine::Get()->Get_Time()->Get_Delta_Seconds() * m_heading_lerp_delta);
	m_normalized_heading = m_heading.Normalize();
	m_last_heading_position = actor->Get_Position();

//	DBG_LOG("Heading(%.2f, %.2f, %.2f) Normal(%.2f, %.2f, %.2f) Length=%f", 
//		m_heading.X, m_heading.Y, m_heading.Z, 
//		m_normalized_heading.X, m_normalized_heading.Y, m_normalized_heading.Z, 
//		m_heading.Length());
}

int GameNetUser::Get_Scoreboard_Rank()
{
	return m_scoreboard_rank;
}

void GameNetUser::Tick(const FrameTime& time)
{
	NetManager* manager = NetManager::Get();

	NetUser::Tick(time);

	// Update posession id.
	if (m_controller_id >= 0)
	{
		ScriptedActor* actor = Game::Get()->Get_Game_Scene()->Find_Actor_By_Unique_ID(m_controller_id);
		if (actor != NULL)
		{
			DBG_LOG("User '%s' gained possession of controller '%i'.", Get_Username().c_str(), m_controller_id);
			m_controller = actor;
			m_controller_id = -1;
			m_last_heading_position = actor->Get_Position();

			m_controller->Get_Event_Listener()->Fire_On_Possession_Synced();
		}
	}

	// Update some general state information.
	if (m_connection != NULL)
	{
		m_state->Ping = (unsigned short)m_connection->Get_Ping();
	}

	// Wait until server is no longer loading to send server sync.
	if (m_game_state == GameNetClient_GameState::SyncingWithServer)
	{
	/*	DBG_LOG("Syncing username='%s' m_is_synced=%i map_pending=%i loading=%i - m_map_load_index=%i/%i profile=%i.", 
			Get_Username().c_str(), 
			m_is_synced, 
			!manager->Is_Map_Load_Pending(),
			GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_MapLoading*>() == NULL,
			m_map_load_index,
			manager->Get_Current_Map_Load_Index(),
			m_profile != NULL);
	*/

		if (m_is_synced == false &&																	// client is not already synced
			!manager->Is_Map_Load_Pending() &&														// no server map load is pending
			GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_MapLoading*>() == NULL &&	// server is not currently loading a map.
			m_map_load_index == manager->Get_Current_Map_Load_Index() &&							// client is on the current server map.
			m_profile != NULL)																		// got profile from user.								
		{
			// Only need to send sync to one local user.
			if (Get_Local_User_Index() == 0)
			{
				GameNetServer* server = GameNetManager::Get()->Game_Server();
				server->Synchronize_User(this);
			}
	
			DBG_LOG("Finished syncronising with user '%s'.", Get_Username().c_str());
			m_is_synced = true;
		}
	}
}

GameNetUserState* GameNetUser::Get_State()
{
	return m_state;
}

void GameNetUser::Set_Map_Load_Index(int index)
{
	m_map_load_index = index;
}

void GameNetUser::Set_Full_Update_Pending(bool value)
{
	m_full_update_pending = value;
}

double GameNetUser::Get_Base_Tick()
{
	return m_base_tick;
}

void GameNetUser::Set_Base_Tick(double value)
{
	m_base_tick = value;
}

bool GameNetUser::Get_Full_Update_Pending()
{
	return m_full_update_pending;
}

void GameNetUser::Set_Game_State(GameNetClient_GameState::Type state, bool set_by_server)
{
	if (m_game_state == state)
	{
		return;
	}

	DBG_LOG("Net user '%i' transitioned to state '%i' (set_by_server=%i).", Get_Net_ID(), state, set_by_server);
	m_game_state = state;

	// If server changed game state for this user we may
	// wish to perform an operation based on it.
	if (set_by_server == true)
	{
		GameNetServer* server = GameNetManager::Get()->Game_Server();

		switch (m_game_state)
		{
		case GameNetClient_GameState::Idle:
			{
				// Nothing to do here.
				break;
			}
		case GameNetClient_GameState::Loading:
			{
				DBG_LOG("Net user '%i' is loading map load index '%i'.", Get_Net_ID(), m_map_load_index);
				break;
			}
		case GameNetClient_GameState::SendingClientInfo:
			{
				// TODO: STUBBED
				break;
			}
		case GameNetClient_GameState::SyncingWithServer:
			{
				m_is_synced = false;
				break;
			}
		case GameNetClient_GameState::WaitingForStart:
			{
				// TODO: We don't do any waiting for start at the moment.
				break;
			}
		case GameNetClient_GameState::InGame:
			{
				// Let scripts know the user has joined the game, so they can setup teams etc.
				ScriptEventListener::Fire_On_User_Ready(m_script_object);
				break;
			}
		}
	}
}

void GameNetUser::Set_Pending_Profile(Profile* profile)
{
	SAFE_DELETE(m_pending_profile);
	m_pending_profile = profile;
}

void GameNetUser::Set_Profile(Profile* profile)
{
	SAFE_DELETE(m_profile);
	m_profile = profile;
	m_profile_change_counter++;

	if (profile != NULL)
	{
		DBG_LOG("Setting profile for user %i", Get_Net_ID());
	}
	else
	{
		DBG_LOG("Clearing profile for user %i", Get_Net_ID());
	}
}

void GameNetUser::Apply_Pending_Profile()
{
	if (m_pending_profile == NULL)
		return;

	DBG_LOG("Applying pending profile update for user %i", Get_Net_ID());

	SAFE_DELETE(m_profile);
	m_profile = m_pending_profile;
	m_pending_profile = NULL;
	m_profile_change_counter++;

	// Send update to all users.
	std::vector<NetUser*> users = NetManager::Get()->Get_Net_Users();
	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		if (!user->Get_Online_User()->Is_Local())
		{
			NetPacket_C2S_ProfileUpdate packet;
			packet.User_ID		= Get_Net_ID();
			packet.Profile_Data = ProfileManager::Get()->Profile_To_Buffer(m_profile);
			user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

			DBG_LOG("Sending profile of user %i to user %i.", Get_Net_ID(), user->Get_Net_ID());
		}
	}
}

Profile* GameNetUser::Get_Profile()
{
	return m_profile;
}

Profile* GameNetUser::Get_Local_Profile()
{
	if (Get_Online_User()->Is_Local())
	{
		return ProfileManager::Get()->Get_Profile(Get_Online_User()->Get_Profile_Index());
	}
	return NULL;
}

void GameNetUser::Restart()
{
	DBG_LOG("GameNetUser::Restart - %s", Get_Username().c_str());

//	m_is_synced = false;
//	m_controller = NULL;
//	m_controller_id = -1;
//	m_game_state = GameNetClient_GameState::LeftGame;
}

void GameNetUser::LeftGame()
{
	DBG_LOG("GameNetUser::LeftGame - %s", Get_Username().c_str());
	m_game_state = GameNetClient_GameState::LeftGame;
}

void GameNetUser::Reset_On_Map_Load()
{
	DBG_LOG("GameNetUser::Reset_On_Map_Load - %s", Get_Username().c_str());
	m_profile = NULL;

	m_is_synced = false;
	m_controller = NULL;
	m_controller_id = -1;
	m_game_state = GameNetClient_GameState::LeftGame;

	if (Get_Online_User()->Is_Local())
	{
		m_initial_level = ProfileManager::Get()->Get_Profile(Get_Online_User()->Get_Profile_Index())->Level;
	}
}

bool GameNetUser::Is_Synced()
{
	return m_is_synced;
}

GameNetClient_GameState::Type GameNetUser::Get_Game_State()
{
	return m_game_state;
}	

ScriptedActor* GameNetUser::Get_Controller()
{
	return m_controller;
}

void GameNetUser::Set_Controller_ID(int id)
{
	// If server, replicate change.
	GameNetServer* server = GameNetManager::Get()->Game_Server();
	if (server != NULL)
	{
		NetPacket_S2C_UserPossessionChanged packet;
		packet.User_ID	= Get_Net_ID();
		packet.Actor_ID = id;

		server->Get_Connection()->Broadcast(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
	
		m_controller_id = -1;
		m_controller = Game::Get()->Get_Game_Scene()->Find_Actor_By_Unique_ID(id);
		
		DBG_LOG("Gave user '%s' possession of controller '%i'.", Get_Username().c_str(), id);
	}
	else
	{
		if (id < 0)
		{
			DBG_LOG("User '%s' lost possession of controller '%i'.", Get_Username().c_str(), id);
			m_controller = NULL;
			m_controller_id = -1;
		}
		else
		{			
			m_controller_id = id;
		}
	}
}

void GameNetUser::Recieve_State_Update(NetPacket_C2S_ClientUserStateUpdate* packet)
{
	Rect2D old_view = m_last_client_state_packet.Viewport;
	Rect2D new_view = packet->Viewport;

	Vector2 velocity = Vector2(new_view.X, new_view.Y) - Vector2(old_view.X, old_view.Y);
	if (velocity.Length() < outlier_viewport_threshold)
	{
		m_viewport_velocity_buffer.Push(velocity);
	}

	if (m_profile != NULL)
	{
		m_profile->Level = packet->Level;
	}

	m_last_client_state_packet = *packet;
	m_last_client_state_recieve_time = Platform::Get()->Get_Ticks();
}

Rect2D GameNetUser::Get_Estimated_Viewport()
{
	// If local just return the active cameras viewport.
	if (Get_Online_User()->Is_Local())
	{
		if (Game::Get()->Get_Game_Mode()->Is_PVP())
		{
			int user_index = Get_Online_User()->Get_Local_Player_Index();
			return Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + user_index))->Get_Bounding_Viewport();
		}
		else
		{
			return Game::Get()->Get_Camera(CameraID::Game1)->Get_Bounding_Viewport();
		}
	}

	// Work out average velocity.
	Vector2 avg_velocity(0.0f, 0.0f);
	for (int i = 0; i < m_viewport_velocity_buffer.Size(); i++)
	{
		const Vector2 sample = m_viewport_velocity_buffer.Get(i);
		avg_velocity.X += sample.X;
		avg_velocity.Y += sample.Y;
	}
	avg_velocity.X /= m_viewport_velocity_buffer.Size();
	avg_velocity.Y /= m_viewport_velocity_buffer.Size();

	// Work out rough latency since last viewport was recieved.
	double time_since_last_state = Platform::Get()->Get_Ticks() - m_last_client_state_recieve_time;
	double latency = (m_state->Ping * 0.5f) + time_since_last_state;

	// Calculate roughly where the viewport is now.
	return Rect2D(
		m_last_client_state_packet.Viewport.X + avg_velocity.X,
		m_last_client_state_packet.Viewport.Y + avg_velocity.Y,
		m_last_client_state_packet.Viewport.Width,
		m_last_client_state_packet.Viewport.Height		
	);
}