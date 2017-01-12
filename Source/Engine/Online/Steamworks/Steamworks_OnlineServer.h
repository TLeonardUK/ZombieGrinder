// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_SERVER_
#define _ENGINE_STEAMWORKS_ONLINE_SERVER_

#include "Engine/Online/OnlineServer.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

class Steamworks_OnlineServer : public OnlineServer 
{
	MEMORY_ALLOCATOR(Steamworks_OnlineServer, "Network");

private:
	Steamworks_OnlinePlatform* m_platform;
		
	OnlineServer_State::Type m_state;
	OnlineServer_Error::Type m_error;

	bool m_vac_secured;

	CSteamID m_server_steamid;
	
	int m_max_players;
	std::string m_server_name;
	std::string m_map_name;
	bool m_private_game;

	bool m_info_dirty;

	bool m_dedicated;

	STEAM_GAMESERVER_CALLBACK(Steamworks_OnlineServer, Callback_SteamServersConnected, SteamServersConnected_t, m_callback_steam_servers_connected);
	STEAM_GAMESERVER_CALLBACK(Steamworks_OnlineServer, Callback_SteamServersConnectFailure, SteamServerConnectFailure_t, m_callback_steam_servers_connect_failure);
	STEAM_GAMESERVER_CALLBACK(Steamworks_OnlineServer, Callback_SteamServersDisconnected, SteamServersDisconnected_t, m_callback_steam_servers_disconnected);
	STEAM_GAMESERVER_CALLBACK(Steamworks_OnlineServer, Callback_PolicyResponse, GSPolicyResponse_t, m_callback_policy_response);

protected:
	
	// Change state.
	void Set_State(OnlineServer_State::Type state);

	// Change error.
	void Set_Error(OnlineServer_Error::Type error);

	// Server info stuff.
	void Update_Server_Info();

public:
	Steamworks_OnlineServer(Steamworks_OnlinePlatform* platform);
	~Steamworks_OnlineServer();

	CSteamID Get_Server_SteamID();

	bool Init();

	void Tick(const FrameTime& time);
		
	void Set_Server_Info(int max_players, const char* map_name, bool private_game);

	OnlineServer_State::Type Get_State();
	OnlineServer_Error::Type Get_Error();
	std::string Get_Error_Message(OnlineServer_Error::Type error);

	std::string Get_Server_Name();
	std::string Get_Host_Name();
	void Flag_Server_Info_Dirty();

};

#endif

