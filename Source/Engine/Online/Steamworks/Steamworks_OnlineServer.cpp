// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlineServer.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Version.h"

extern "C" void __cdecl SteamAPI_ServerDebugCallback(int nSeverity, const char *pchDebugText)
{
	DBG_LOG("[Steam Server] %s", pchDebugText);
}

Steamworks_OnlineServer::Steamworks_OnlineServer(Steamworks_OnlinePlatform* platform)
	: m_platform(platform)
	, m_callback_steam_servers_connected(this, &Steamworks_OnlineServer::Callback_SteamServersConnected)
	, m_callback_steam_servers_disconnected(this, &Steamworks_OnlineServer::Callback_SteamServersDisconnected)
	, m_callback_steam_servers_connect_failure(this, &Steamworks_OnlineServer::Callback_SteamServersConnectFailure)
	, m_callback_policy_response(this, &Steamworks_OnlineServer::Callback_PolicyResponse)
	, m_state(OnlineServer_State::Idle)
	, m_error(OnlineServer_Error::NONE)
	, m_vac_secured(false)
	, m_max_players(16)
	, m_server_name("")
	, m_map_name("")
	, m_private_game(false)
	, m_dedicated(false)
	, m_info_dirty(false)
{
} 

Steamworks_OnlineServer::~Steamworks_OnlineServer()
{	
	DBG_LOG("Disposing of online server ...");

	if (SteamGameServer())
	{
		SteamGameServer()->EnableHeartbeats(false);
		SteamGameServer()->LogOff();

		// Wait until we are logged off.
		//while (SteamGameServer()->BLoggedOn())
		//{
		//	Platform::Get()->Sleep(1.0f);
		//}

		//SteamGameServerUtils()->SetWarningMessageHook(NULL);
		SteamGameServer_Shutdown();
	}
}

CSteamID Steamworks_OnlineServer::Get_Server_SteamID()
{
	return m_server_steamid;
}

bool Steamworks_OnlineServer::Init()
{
	m_state = OnlineServer_State::Initializing;
	m_error = OnlineServer_Error::NONE;
	m_vac_secured = false;
	m_server_name = *EngineOptions::server_name;
	m_dedicated = (*EngineOptions::server == true);

	DBG_LOG("[Steam Server] Setting up online platform as server.");
		
	// Get IP to bind to.
	unsigned int ip = INADDR_ANY;
	if (*EngineOptions::server_ip != "")
	{
		NetAddress address;
		if (NetAddress::From_String((*EngineOptions::server_ip).c_str(), address))
		{
			ip = address.Get_IP();
			DBG_LOG("[Steam Server] Binding to server IP '%s'.", (*EngineOptions::server_ip).c_str());
		}
		else
		{
			DBG_LOG("[Steam Server] Invalid server-ip '%s' specified in options, using any address.", (*EngineOptions::server_ip).c_str());
		}
	}

	// Secure or not?
	EServerMode mode = eServerModeAuthenticationAndSecure;

	if (*EngineOptions::server_secure == false)
	{
		mode = eServerModeAuthentication;
	}

	// Initialize steam-server! Wooooo
	DBG_LOG("[Steam Server] Calling SteamGameServer_Init(%s, %i, %i, %i, %i, %s).", ip, *EngineOptions::server_auth_port, *EngineOptions::server_port, *EngineOptions::server_master_port, mode, (*EngineOptions::server_steam_version).c_str());
	if (!SteamGameServer_Init(ip, *EngineOptions::server_auth_port, *EngineOptions::server_port, *EngineOptions::server_master_port, mode, (*EngineOptions::server_steam_version).c_str()))
	{
		DBG_LOG("[Steam Server] SteamGameServer_Init(%i, %i, %i, %i, %i, %s) return failure.", ip, *EngineOptions::server_auth_port, *EngineOptions::server_port, *EngineOptions::server_master_port, mode, (*EngineOptions::server_steam_version).c_str());
		return false;
	}
	
	// Bind the warning hook.
	SteamGameServerUtils()->SetWarningMessageHook(&SteamAPI_ServerDebugCallback);

	// Setup initial properties.
	SteamGameServer()->SetModDir((*EngineOptions::server_game_dir).c_str());
	SteamGameServer()->SetProduct((*EngineOptions::product_name).c_str());
	SteamGameServer()->SetGameDescription((*EngineOptions::product_description).c_str());
	SteamGameServer()->SetDedicatedServer(m_dedicated);

	// Initiate login.
	SteamGameServer()->LogOnAnonymous();

	// Start headerbeats.
	SteamGameServer()->EnableHeartbeats(true);

	// Get the temporary steamid (not a universal id until we are logged in)
	m_server_steamid = SteamGameServer()->GetSteamID();
	DBG_LOG("[Steam Server] Logged in with (temporary) server-id %llu", m_server_steamid.ConvertToUint64());

	return true;
}

void Steamworks_OnlineServer::Tick(const FrameTime& time)
{
	switch (m_state)
	{
	case OnlineServer_State::Active:
		{
			if (m_info_dirty == true)
			{
				Update_Server_Info();

				m_info_dirty = false;
			}
			break;
		}
	}

	// Invoke steam callbacks.
	SteamGameServer_RunCallbacks();
}

void Steamworks_OnlineServer::Update_Server_Info()
{
	SteamGameServer()->SetMaxPlayerCount(m_max_players);
	SteamGameServer()->SetPasswordProtected(false);
	SteamGameServer()->SetServerName(m_server_name.c_str());
	SteamGameServer()->SetBotPlayerCount(0); 
	SteamGameServer()->SetMapName(m_map_name.c_str());

	if (m_private_game == true)
	{
		SteamGameServer()->SetGameTags(StringHelper::Format("private,protocol_%i", NetManager::Get()->Get_Net_Version()).c_str());
	}
	else
	{
		SteamGameServer()->SetGameTags(StringHelper::Format("protocol_%i", NetManager::Get()->Get_Net_Version()).c_str());
	}

	// Update all the players names/scores
	Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();
	std::vector<NetUser*>& users = NetManager::Get()->Get_Net_Users();

	// If ticketing is enabled, none of this will work correctly.
#if !defined(OPT_DISABLE_TICKETING) && !defined(OPT_STEAM_BSD_SOCKETS)
	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		NetUser* net_user = *iter;		
		Steamworks_OnlineUser* user = static_cast<Steamworks_OnlineUser*>(net_user->Get_Online_User());

		// We can only update information on the actual steam-user, not local-users.
		if (net_user->Get_Local_User_Index() == 0)
		{
			if (!SteamGameServer()->BUpdateUserData(user->Get_SteamID(), net_user->Get_Username().c_str(), net_user->Get_Score()))
			{
			//	DBG_LOG("BUpdateUserData failed for user '%s'.", net_user->Get_Username().c_str());
			}
		}
	}
#endif
}

void Steamworks_OnlineServer::Set_Server_Info(int max_players, const char* map_name, bool private_game)
{
	m_max_players = max_players;
	m_map_name = map_name;
	m_private_game = private_game;
	m_info_dirty = true;
}

void Steamworks_OnlineServer::Callback_SteamServersConnected(SteamServersConnected_t* params)
{
	DBG_LOG("[Steam Server] Connected to steam servers.");
	
	// Get the universal id.
	m_server_steamid = SteamGameServer()->GetSteamID();
	DBG_LOG("[Steam Server] Logged in with (universal) server-id %llu", m_server_steamid.ConvertToUint64());

	if (*EngineOptions::server_secure == false)
	{
		Set_State(OnlineServer_State::Active);
	}
	else
	{
		Set_State(OnlineServer_State::Securing);
	}
}

void Steamworks_OnlineServer::Callback_SteamServersConnectFailure(SteamServerConnectFailure_t* params)
{
	DBG_LOG("[Steam Server] Failed to create connection to steam servers (reason=0x%08x).", params->m_eResult);

	Set_Error(OnlineServer_Error::Offline);
}

void Steamworks_OnlineServer::Callback_SteamServersDisconnected(SteamServersDisconnected_t* params)
{
	DBG_LOG("[Steam Server] Lost connection to steam servers (reason=0x%08x).", params->m_eResult);

	Set_Error(OnlineServer_Error::Offline);
}

void Steamworks_OnlineServer::Callback_PolicyResponse(GSPolicyResponse_t* params)
{
	if (params->m_bSecure)
	{
		DBG_LOG("[Steam Server] *** VAC Secured ***");
		m_vac_secured = true;
	}
	else
	{
		DBG_LOG("[Steam Server] *** Not VAC Secured ***");
		m_vac_secured = false;
	}
	
	Set_State(OnlineServer_State::Active);
}

void Steamworks_OnlineServer::Set_State(OnlineServer_State::Type state)
{
	DBG_LOG("[Steam Server] Changing state to '%i'.", state);
	m_state = state;
}

void Steamworks_OnlineServer::Set_Error(OnlineServer_Error::Type error)
{
	DBG_LOG("[Steam Server] Changing error to '%i'.", error);
	m_error = error;
	Set_State(OnlineServer_State::Error);
}

OnlineServer_State::Type Steamworks_OnlineServer::Get_State()
{
	return m_state;
}

OnlineServer_Error::Type Steamworks_OnlineServer::Get_Error()
{
	return m_error;
}

std::string Steamworks_OnlineServer::Get_Error_Message(OnlineServer_Error::Type error)
{	
	switch (error)
	{
		case OnlineServer_Error::Unknown:					return "#matching_error_internal_error";
		case OnlineServer_Error::Invalid_State:				return "#matching_error_invalid_state";
		case OnlineServer_Error::Offline:					return "#matching_error_offline";
	}

	DBG_ASSERT(false);
	return "matching_error_internal_error";
}

std::string Steamworks_OnlineServer::Get_Server_Name()
{
	return m_server_name;
}

std::string Steamworks_OnlineServer::Get_Host_Name()
{
	if (m_dedicated == true)
	{
		NetAddress address(SteamGameServer()->GetPublicIP(), 0);
		return address.To_String();
	}
	else
	{
		Steamworks_OnlineUser* sw_user = dynamic_cast<Steamworks_OnlineUser*>(Steamworks_OnlinePlatform::Get()->Get_Initial_User());
		return sw_user->Get_Username();
	}
}

void Steamworks_OnlineServer::Flag_Server_Info_Dirty()
{
	m_info_dirty = true;
}