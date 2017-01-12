// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetServer.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineServer.h"
#include "Engine/Online/OnlineMatching.h"

#include "Engine/Network/Packets/EnginePackets.h"

#include "Engine/IO/StreamFactory.h"

#include "Generic/Math/Random.h"

#include "Engine/Engine/EngineOptions.h"

NetServer::NetServer()
	: m_reset_settings(true)
	, m_private_game(false)
	, m_callback_recieve_reservation(this, &NetServer::Callback_RecievedReserveRequest, false, NULL)
	, m_callback_recieve_name_changed(this, &NetServer::Callback_RecievedNameChanged, false, NULL)
	, m_map_load_index(0)
	, m_joining_disabled(false)
	, m_last_spawn_time(0.0)
{
	m_last_auto_change_time = Platform::Get()->Get_Ticks();
}

void NetServer::Set_Joining_Enabled(bool enabled)
{
	m_joining_disabled = !enabled;
}

NetServer::~NetServer()
{
	// Dispose of disconnected users.
	std::vector<NetUser*> net_users = NetManager::Get()->Get_Net_Users();	
	for (unsigned int i = 0; i < net_users.size(); i++)
	{
		NetUser* user = net_users.at(i);
		NetManager::Get()->Delete_Net_User(user);
	}

	// Dispose connection.
	if (m_listen_connection != NULL)
	{
		// NetManager will deal with disposal.
		m_listen_connection->Dispose();
		m_listen_connection = NULL;
	}

	// Force all connections to be disposed.
	NetManager::Get()->Poll_Connections(true);
}	

bool NetServer::Is_Secure()
{
	return m_secure;
}

bool NetServer::Init()
{
	m_listen_connection = NetConnection::Create();
	if (m_listen_connection == NULL)
	{
		return false;
	}

	// Listen on the server port/address.
	NetAddress local(0, *EngineOptions::server_port);
	if ((*EngineOptions::server_ip) != "")
	{
		NetAddress ip_output;
		if (NetAddress::From_String((*EngineOptions::server_ip).c_str(), ip_output))
		{
			local = NetAddress(ip_output.Get_IP(), local.Get_Port());
		}
	}

	DBG_LOG("Listening for connections on interface '%s'.", local.To_String().c_str());
	if (!m_listen_connection->Listen(local))
	{
		DBG_LOG("Failed to listen on interface.");
		return false;
	}

	// Grab some basic settings.
	m_max_players		= *EngineOptions::server_max_players;
	m_reserved_lobby_id = 0;
	m_reserved_lobby_time = 0.0f;
	m_private_game		= false;
	m_net_id_counter	= 0;
	m_secure			= *EngineOptions::server_secure;

	// Load any bans saved locally.
	Load_Bans();

	return true;
}

u64 NetServer::Get_Reserved_Lobby_ID()
{
	return m_reserved_lobby_id;
}

double NetServer::Get_Reserved_Lobby_Time()
{
	return m_reserved_lobby_time;
}

void NetServer::Tick(const FrameTime& time)
{
	std::vector<NetUser*> net_users = NetManager::Get()->Get_Net_Users();

#ifndef MASTER_BUILD

	// Spawn debug clients?
	int client_count = *EngineOptions::spawn_clients;
	double ticks = Platform::Get()->Get_Ticks();
	if (client_count > 0 && (int)net_users.size() < client_count && ticks - m_last_spawn_time > 20 * 1000.0f)
	{
		DBG_LOG("Spawning sub-client");

		if (*EngineOptions::enable_ai || *EngineOptions::enable_local_ai)
		{
			Platform::Get()->Spawn_Process("Game.exe", StringHelper::Format("-client_id %i -enable_ai 1 -connect 0123456789 -client_lifetime %i", net_users.size(), Random::Static_Next(*EngineOptions::client_min_life, *EngineOptions::client_max_life)));
		}
		else
		{
			Platform::Get()->Spawn_Process("Game.exe", StringHelper::Format("-client_id %i -connect 0123456789 -client_lifetime %i", net_users.size(), Random::Static_Next(*EngineOptions::client_min_life, *EngineOptions::client_max_life)));
		}

		m_last_spawn_time = ticks;
	}

	int auto_change_frequency = *EngineOptions::map_auto_change_timer;
	if (auto_change_frequency > 0 && (ticks - m_last_auto_change_time) > auto_change_frequency)
	{
		DBG_LOG("Auto changing map!");

		std::vector<MapFileHandle*> maps = ResourceFactory::Get()->Get_Sorted_Map_Files();
		MapFileHandle* map = maps[Random::Static_Next(0, maps.size() - 1)];
		MapFileHeaderBlock* block = map->Get()->Get_Header();
		Force_Map_Change(block->GUID.c_str(), block->Workshop_ID, Random::Static_Next(), 0);

		m_last_auto_change_time = ticks;
	}


#endif

	// Update server stats with platform master server.
	OnlinePlatform::Get()->Server()->Set_Server_Info(m_max_players, NetManager::Get()->Get_Current_Short_Map_Name().c_str(), m_private_game);

	// Dispose of disconnected users.
	for (std::vector<NetUser*>::iterator iter = m_disconnected_users.begin(); iter != m_disconnected_users.end(); iter++)
	{
		NetUser* user = *iter;
		NetManager::Get()->Delete_Net_User(user);
	}
	m_disconnected_users.clear();

	// Lobby reservation over?
	if (m_reserved_lobby_id != 0)
	{
		m_reserved_lobby_time -= time.Get_Frame_Time();
		if (m_reserved_lobby_time <= 0.0f)
		{
			DBG_LOG("Lobby reservation timed out. Unreserving server.");
			m_reserved_lobby_id = 0;
		}
	}

	// Reset settings when no users.
	if (net_users.size() == 0)
	{
		if (m_reset_settings == false)
		{
			DBG_LOG("No users in server, resettings settings.");

			Reset_Settings();

			m_reset_settings = true;
		}
	}
	else 
	{
		if (m_reset_settings)
		{
			DBG_LOG("First player joined server, restarting map.");

			Restart_Map();

			m_reset_settings = false;
		}
	}
}

void NetServer::Reset_Settings()
{
	m_joining_disabled = false;
	m_private_game = false;
	m_max_players = *EngineOptions::server_max_players;

	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_Short_Name((*EngineOptions::server_map_name).c_str());
	if (handle == NULL)
	{
		handle = ResourceFactory::Get()->Get_Map_File_By_Short_Name((*EngineOptions::server_safe_map_name).c_str());
	}
	DBG_ASSERT(handle != NULL)

	Force_Map_Change(handle->Get()->Get_Header()->GUID.c_str(), handle->Get()->Get_Header()->Workshop_ID, Random::Static_Next(), 0);
}

int NetServer::Total_Player_Slots()
{
	return m_max_players;
}

int NetServer::Available_Player_Slots()
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();
	return (m_max_players - net_users.size());
}

bool NetServer::Is_Connection_Banned(NetConnection* connection)
{
	u64 id = connection->Get_Online_User()->Get_User_ID();

	for (std::vector<NetServerBan>::iterator iter = m_banned_ids.begin(); iter != m_banned_ids.end(); iter++)
	{
		NetServerBan& ban = *iter;
		if (ban.id == id)
		{
			return true;
		}
	}

	return false;
}

bool NetServer::Is_Connection_Duplicate(NetConnection* connection)
{	
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		NetUser* user = *iter;
		NetConnection* other = user->Get_Connection();

		if (other != NULL && 
			other->Get_Online_User() == connection->Get_Online_User())
		{
			return true;
		}
	}

	return false;
}

bool NetServer::Is_Connection_Part_Of_Reservation(NetConnection* connection)
{
	if (m_joining_disabled)
	{
		DBG_LOG("Joining is disabled, player is not part of reservation.");
		return false;
	}

	if (m_reserved_lobby_id == 0)
	{
		DBG_LOG("No reserved lobby, player is 'part of reservation'.");
		return true;
	}
	else if (connection->Get_Source_Lobby_ID() == m_reserved_lobby_id)
	{
		DBG_LOG("Player is part of reserved lobby.");
		return true;
	}

	DBG_LOG("Unknown issue, player is not part of reserved lobby.");
	return false;
}

void NetServer::On_Connected(NetConnection* connection, NetPacket_C2S_Connect* packet)
{
	DBG_LOG("On_Connection invoked with %i users.", packet->Local_User_Count);

	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();
	
	NetPacket_S2C_InitialState state_packet;

	NetPacket_S2C_UserListUpdate update_packet;
	update_packet.Update_Type = UserListUpdateType::Join;

	NetUser* main_user = NULL;

	for (int i = 0; i < packet->Local_User_Count; i++)
	{
		NetUser* user = NetManager::Get()->Create_Net_User(connection, i);
		user->Set_Net_ID((m_net_id_counter++) % 0xFFFF);
		user->Set_Username(packet->Usernames[i]);
#ifdef OPT_PREMIUM_ACCOUNTS
		user->Get_Online_User()->Set_Premium(packet->Premium_Account);
#endif
		user->Get_Online_User()->Set_Short_Country_Name(packet->Short_Country_Name.c_str());

		if (main_user == NULL)
		{
			main_user = user;
		}

		state_packet.Net_IDs[i] = user->Get_Net_ID();

		DBG_LOG("'%s' joined the game (net id = %u).", user->Get_Username().c_str(), (u32)user->Get_Net_ID());
		On_User_Joined(user);

		UserListState state;
		state.Net_ID		= user->Get_Net_ID();
		state.Username		= user->Get_Username();
		state.Local_Index	= i;
		state.Short_Country_Name = user->Get_Online_User()->Get_GeoIP_Result().CountryShortName;
#ifdef OPT_PREMIUM_ACCOUNTS
		state.Premium		= user->Get_Online_User()->Get_Premium();
#endif
		state.User_ID		= user->Get_Online_User()->Get_User_ID();
		update_packet.States.push_back(state);
	}

	if (packet->Local_User_Count == 0)
	{
		DBG_LOG("Connection is query connection, no local users.");
	}
	else
	{
		// Send pre-map change information.
		SendPreInitialMapChangeInfo(main_user);

		// Broadcast join messages to all other users.
		m_listen_connection->Broadcast(&update_packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true, connection);

		// Send this user a complete list.
		NetPacket_S2C_UserListUpdate full_packet;
		full_packet.Update_Type = UserListUpdateType::Full;

		for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
		{
			NetUser* user = *iter;
			
			UserListState state;
			state.Net_ID		= user->Get_Net_ID();
			state.Username		= user->Get_Username();
			state.Local_Index	= user->Get_Local_User_Index();
			state.Short_Country_Name = user->Get_Online_User()->Get_GeoIP_Result().CountryShortName;
#ifdef OPT_PREMIUM_ACCOUNTS
			state.Premium		= user->Get_Online_User()->Get_Premium();
#endif
			state.User_ID		= user->Get_Online_User()->Get_User_ID();
			full_packet.States.push_back(state);
		}
		connection->Send(&full_packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

		// Send them a message telling them what map we are on.
		connection->Send(&state_packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

		// Now tell them what map we are on.
		NetPacket_S2C_ChangeMap map_packet;
		map_packet.Map_GUID = NetManager::Get()->Get_Current_Map_GUID();
		map_packet.Map_Workshop_ID = NetManager::Get()->Get_Current_Map_Workshop_ID();
		map_packet.Map_Seed = NetManager::Get()->Get_Current_Map_Seed();
		map_packet.Map_Load_Index = NetManager::Get()->Get_Current_Map_Load_Index();
		map_packet.Map_Dungeon_Level_Index = NetManager::Get()->Get_Current_Map_Dungeon_Level();

		connection->Send(&map_packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
	}
}

void NetServer::On_Disconnected(NetConnection* connection)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	NetPacket_S2C_UserListUpdate update_packet;
	update_packet.Update_Type = UserListUpdateType::Leave;

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		NetUser* user = *iter;
		if (user->Get_Connection() == connection)
		{			
			DBG_LOG("'%s' left the game (net id = %u).", user->Get_Username().c_str(), (u32)user->Get_Net_ID());
  			m_disconnected_users.push_back(user); 
		
			On_User_Left(user);

			UserListState state;
			state.Net_ID		= user->Get_Net_ID();
			state.Username		= user->Get_Username();
			state.Short_Country_Name = user->Get_Online_User()->Get_GeoIP_Result().CountryShortName;
			state.Local_Index	= user->Get_Local_User_Index();
#ifdef OPT_PREMIUM_ACCOUNTS
			state.Premium		= user->Get_Online_User()->Get_Premium();
#endif
			state.User_ID		= user->Get_Online_User()->Get_User_ID();
			update_packet.States.push_back(state);

			continue;
		}
	}

	// Broadcast leave messages to all other users.
	m_listen_connection->Broadcast(&update_packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true, connection);
}

bool NetServer::Callback_RecievedReserveRequest(NetPacket_C2S_ReserveServer* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();

	NetConnection* connection = packet->Get_Recieved_From();

	NetPacket_S2C_ReservationResult output;
	output.Result = false;

#ifdef OPT_PREMIUM_ACCOUNTS
	if (*EngineOptions::server_premium)
	{
		DBG_LOG("User attempted to reserve premium, DENIED!");
	}
	else
#endif
		if (m_reserved_lobby_id != 0)
	{
		DBG_LOG("User attempted to reserve server, but it's already reserved.");
		return false;
	}
	else if (net_users.size() > 0)
	{		
		DBG_LOG("User attempted to reserve server, but we already have players in us (hehe).");
	}
	else if (packet->MaxPlayers > *EngineOptions::server_max_players)
	{
		DBG_LOG("User attempted to reserve server, but asked for more player slots than we allow.");
	}
	else
	{		
		MapFileHandle* mapHandle = ResourceFactory::Get()->Get_Map_File_By_GUID(packet->MapGUID.c_str());
		if (mapHandle == NULL)
		{
			DBG_LOG("Rejected attempt to reserver server for lobby, we don't have the map installed that the users want!");
		}
		else
		{
			DBG_LOG("User reserved our server for lobby id '%llu'.", packet->LobbyID);
			m_reserved_lobby_id		= packet->LobbyID;
			m_reserved_lobby_time	= (float)reserved_lobby_timeout;

			m_max_players	= packet->MaxPlayers;
			m_private_game  = packet->Private;
			Force_Map_Change(packet->MapGUID.c_str(), packet->MapWorkshopID, Random::Static_Next(), 0);

			output.Result = true;
		}
	}

	connection->Send(&output, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
	
	return true;
}

void NetServer::Reserve(u64 lobby_id, OnlineMatching_LobbySettings* settings)
{
	DBG_LOG("Reserved server for lobby id '%llu'.", lobby_id);

	m_reserved_lobby_id		= lobby_id;
	m_reserved_lobby_time	= (float)reserved_lobby_timeout;

	m_max_players			= settings->MaxPlayers;
	m_private_game			= settings->Private;
	m_secure				= settings->Secure;

	int seed = Random::Static_Next();

#ifndef MASTER_BUILD
	if ((*EngineOptions::map_seed) != "")
	{
		seed = strtol((*EngineOptions::map_seed).substr(2).c_str(), NULL, 16);
	}
#endif

	Force_Map_Change(settings->MapGUID.c_str(), settings->MapWorkshopID, seed, 0);
}

void NetServer::Kick_User(NetUser* user)
{
	DBG_LOG("Attempting to kick user '%s'.", user->Get_Username().c_str());

	NetPacket_S2C_Kick packet;
	user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
}

void NetServer::Ban_User(NetUser* user)
{
	DBG_LOG("Attempting to ban user '%s'.", user->Get_Username().c_str());

	NetServerBan ban;
	ban.id = user->Get_Online_User()->Get_User_ID();
	ban.username = user->Get_Username();

	m_banned_ids.push_back(ban);
	Save_Bans();

	Kick_User(user);
}

void NetServer::Force_Map_Change(const char* guid, u64 workshop_id, int seed, int dungeon_level_index)
{
	DBG_LOG("Server is changing map to '%s' (workshopid=%llu, seed=0x%08x, index=%i, dungeon_level=%i).", guid, workshop_id, seed, m_map_load_index, dungeon_level_index);

	NetManager::Get()->Change_Map_By_GUID(guid, workshop_id, seed, m_map_load_index, dungeon_level_index);
	
	// Tell all clients what we are up to!
	NetPacket_S2C_ChangeMap map_packet;
	map_packet.Map_GUID					= guid;
	map_packet.Map_Workshop_ID			= workshop_id;
	map_packet.Map_Seed					= seed;
	map_packet.Map_Load_Index			= m_map_load_index;
	map_packet.Map_Dungeon_Level_Index	= dungeon_level_index;
	m_listen_connection->Broadcast(&map_packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	// Reset users for new map load.	
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();
	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		NetUser* user = *iter;
		user->Reset_On_Map_Load();
	}

	m_map_load_index++;
}

void NetServer::Save_Bans()
{
	Stream* stream = StreamFactory::Open("bans.ini", StreamMode::Write);
	if (stream == NULL)
	{
		return;
	}

	for (std::vector<NetServerBan>::iterator iter = m_banned_ids.begin(); iter != m_banned_ids.end(); iter++)
	{
		NetServerBan id = *iter;
		stream->WriteLine(id.username.c_str());
		stream->WriteLine(StringHelper::To_String(id.id).c_str());
	}

	SAFE_DELETE(stream);
}

void NetServer::Load_Bans()
{
	Stream* stream = StreamFactory::Open("bans.ini", StreamMode::Read);
	if (stream == NULL)
	{
		return;
	}

	m_banned_ids.clear();

	while (!stream->IsEOF())
	{
		std::string username = stream->ReadLine();
		const char* line = stream->ReadLine();

#if defined(LANGUAGE_STANDARD_CPP11) || defined(PLATFORM_WIN32)
		u64 id = std::stoull(line);
#else
		u64 id = strtoull(line, NULL, 10);
#endif

		DBG_LOG("Loaded Ban: %s", username.c_str());

		NetServerBan ban;
		ban.id = id;
		ban.username = username;
		m_banned_ids.push_back(ban);
	}

	SAFE_DELETE(stream);
}

void NetServer::Unban_User(u64 id)
{
	for (std::vector<NetServerBan>::iterator iter = m_banned_ids.begin(); iter != m_banned_ids.end(); iter++)
	{
		NetServerBan& ban = *iter;
		if (ban.id == id)
		{
			m_banned_ids.erase(iter);
			break;
		}
	}

	Save_Bans();
}

int NetServer::Map_Load_Count()
{
	return m_map_load_index;
}

void NetServer::Restart_Map()
{
	Force_Map_Change(
		NetManager::Get()->Get_Current_Map_GUID().c_str(), 
		NetManager::Get()->Get_Current_Map_Workshop_ID(), 
		NetManager::Get()->Get_Current_Map_Seed(),
		NetManager::Get()->Get_Current_Map_Dungeon_Level());
}

NetConnection* NetServer::Get_Connection()
{
	return m_listen_connection;
}

bool NetServer::Callback_RecievedNameChanged(NetPacket_C2S_NameChanged* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();
	
	NetConnection* connection = packet->Get_Recieved_From();

	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		NetUser* user = *iter;
		if (user->Get_Net_ID() == packet->Net_ID)
		{			
			DBG_LOG("Recieved client name change for user '%s' -> '%s' (net id = %u).", user->Get_Username().c_str(), packet->Username.c_str(), (u32)user->Get_Net_ID());

			if (user->Get_Connection() != connection)
			{		
				DBG_LOG("Name change request was not for a connections local users. Hacking attempt?");
				break;
			}
			
			On_User_Name_Change(user, user->Get_Display_Username(), user->Get_Display_Username(packet->Username));

			// Change username.
			user->Set_Username(packet->Username);

			// Tell everyone else this name change occured!
			m_listen_connection->Broadcast(packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true, connection);

			break;
		}
	}

	return true;
}

