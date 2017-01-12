// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"

NetClient::NetClient()
	: m_callback_recieve_user_list_update(this, &NetClient::Callback_RecievedUserListUpdate, false, NULL)
	, m_callback_recieve_name_changed(this, &NetClient::Callback_RecievedNameChanged, false, NULL)
	, m_callback_recieve_initial_State(this, &NetClient::Callback_RecievedInitialState, false, NULL)
	, m_callback_recieve_change_map(this, &NetClient::Callback_RecievedChangeMap, false, NULL)
	, m_callback_recieve_kick(this, &NetClient::Callback_RecievedKick, false, NULL)
{
}

NetClient::~NetClient()
{
	if (m_connection != NULL)
	{
		// NetManager will deal with disposal.
		m_connection->Dispose();
		m_connection = NULL;
	}

	// Force all connections to be disposed.
	NetManager::Get()->Poll_Connections();
}	

bool NetClient::Init()
{
	m_state = NetClient_State::Idle;
	m_error = NetClient_Error::NONE;
	
	m_connection = NetConnection::Create();
	if (m_connection == NULL)
	{
		return false;
	}

	return true;
}

void NetClient::Tick_Connecting(const FrameTime& time)
{
	// Connected yet?			
	switch (m_connection->Get_State())
	{
	case NetConnection_State::Connecting:
		{
			// Nothing to do but wait.
			break;
		}
	case NetConnection_State::Connected:
		{
			DBG_LOG("Connected to server '%s' (%s).", m_server.Name.c_str(), m_server.Address.To_String().c_str());
			Set_State(NetClient_State::Connected);
			break;
		}
	case NetConnection_State::Disconnected:
	case NetConnection_State::Disconnecting:
		{
			DBG_LOG("Disconnected from server '%s' (%s).", m_server.Name.c_str(), m_server.Address.To_String().c_str());
			Set_State(NetClient_State::Idle);
			break;
		}
	case NetConnection_State::Error:
		{
			NetConnection_Error::Type error = m_connection->Get_Error();
			DBG_LOG("Encountered unexpected net connection error while connecting %i.", error);

			switch (error)
			{		
				case NetConnection_Error::NONE:					Set_Error(NetClient_Error::NONE);					break;
				case NetConnection_Error::Unknown:				Set_Error(NetClient_Error::Unknown);				break;
				case NetConnection_Error::Unavailable:			Set_Error(NetClient_Error::Unavailable);			break;
				case NetConnection_Error::Timeout:				Set_Error(NetClient_Error::Timeout);				break;

				case NetConnection_Error::Wrong_Version:		Set_Error(NetClient_Error::Wrong_Version);			break;
				case NetConnection_Error::Server_Full:			Set_Error(NetClient_Error::Server_Full);			break;
#ifdef OPT_PREMIUM_ACCOUNTS
				case NetConnection_Error::Server_Premium:		Set_Error(NetClient_Error::Server_Premium);			break;
#endif
				case NetConnection_Error::Banned:				Set_Error(NetClient_Error::Banned);					break;
				case NetConnection_Error::Kicked:				Set_Error(NetClient_Error::Kicked);					break;
				case NetConnection_Error::Lost_Connection:		Set_Error(NetClient_Error::Lost_Connection);		break;
				case NetConnection_Error::Duplicate_Connection:	Set_Error(NetClient_Error::Duplicate_Connection);	break;
				case NetConnection_Error::Transmission_Error:	Set_Error(NetClient_Error::Transmission_Error);		break;
				case NetConnection_Error::Server_Reserved:		Set_Error(NetClient_Error::Server_Reserved);		break;
				case NetConnection_Error::Invalid_Ticket:		Set_Error(NetClient_Error::Invalid_Ticket);			break;
				case NetConnection_Error::Client_Mod_Missmatch:	Set_Error(NetClient_Error::Client_Mod_Missmatch);	Set_Client_Missing_Mods(m_connection->Get_Client_Missing_Mods()); break;
				case NetConnection_Error::Server_Mod_Missmatch:	Set_Error(NetClient_Error::Server_Mod_Missmatch);	Set_Server_Missing_Mods(m_connection->Get_Server_Missing_Mods()); break;

				default:										Set_Error(NetClient_Error::Unknown);				break;
			}

			break;
		}
	default:
		{
			DBG_LOG("Encountered unexpected net connection state while connecting %i.", m_connection->Get_State());
			Set_Error(NetClient_Error::Unknown);
			break;
		}
	}
}

void NetClient::Tick_Connected(const FrameTime& time)
{
}

void NetClient::On_Name_Changed(OnlineUser* user, std::string old_username, std::string new_username)
{
	if (!user->Is_Local())
	{
		return;
	}

	std::vector<NetUser*> net_users = NetManager::Get()->Get_Net_Users();

	// Have we changed our name? If so we need to inform the server.
	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		NetUser* net_user = *iter;
		if (net_user->Get_Online_User() == user)
		{
			NetPacket_C2S_NameChanged packet;
			packet.Net_ID = net_user->Get_Net_ID();
			packet.Username = user->Get_Username().c_str();

			m_connection->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

			DBG_LOG("Sent name change notification to server for net-user %i ('%s' -> '%s').", packet.Net_ID, old_username.c_str(), new_username.c_str());
		}
	}
}

void NetClient::Tick(const FrameTime& time)
{
	switch (m_state)
	{
	case NetClient_State::Idle:
		{
			break;
		}
	case NetClient_State::Connecting:
		{
			Tick_Connecting(time);
			break;
		}
	case NetClient_State::Connected:
		{
			switch (m_connection->Get_State())
			{
			case NetConnection_State::Connected:
				{
					Tick_Connected(time);
					break;
				}
			case NetConnection_State::Disconnected:
			case NetConnection_State::Disconnecting:
			case NetConnection_State::Error:
			default:
				{
					NetConnection_Error::Type error = m_connection->Get_Error();
					DBG_LOG("Encountered unexpected net connection error %i.", error);

					switch (error)
					{		
						case NetConnection_Error::NONE:					Set_Error(NetClient_Error::NONE);					break;
						case NetConnection_Error::Unknown:				Set_Error(NetClient_Error::Unknown);				break;
						case NetConnection_Error::Unavailable:			Set_Error(NetClient_Error::Unavailable);			break;
						case NetConnection_Error::Timeout:				Set_Error(NetClient_Error::Timeout);				break;

						case NetConnection_Error::Wrong_Version:		Set_Error(NetClient_Error::Wrong_Version);			break;
						case NetConnection_Error::Server_Full:			Set_Error(NetClient_Error::Server_Full);			break;
#ifdef OPT_PREMIUM_ACCOUNTS
						case NetConnection_Error::Server_Premium:		Set_Error(NetClient_Error::Server_Premium);			break;
#endif
						case NetConnection_Error::Banned:				Set_Error(NetClient_Error::Banned);					break;
						case NetConnection_Error::Kicked:				Set_Error(NetClient_Error::Kicked);					break;
						case NetConnection_Error::Lost_Connection:		Set_Error(NetClient_Error::Lost_Connection);		break;
						case NetConnection_Error::Duplicate_Connection:	Set_Error(NetClient_Error::Duplicate_Connection);	break;
						case NetConnection_Error::Transmission_Error:	Set_Error(NetClient_Error::Transmission_Error);		break;
						case NetConnection_Error::Server_Reserved:		Set_Error(NetClient_Error::Server_Reserved);		break;
						case NetConnection_Error::Invalid_Ticket:		Set_Error(NetClient_Error::Invalid_Ticket);			break;
						case NetConnection_Error::Client_Mod_Missmatch:	Set_Error(NetClient_Error::Client_Mod_Missmatch);	Set_Client_Missing_Mods(m_connection->Get_Client_Missing_Mods()); break;
						case NetConnection_Error::Server_Mod_Missmatch:	Set_Error(NetClient_Error::Server_Mod_Missmatch);	Set_Server_Missing_Mods(m_connection->Get_Server_Missing_Mods()); break;

						default:										Set_Error(NetClient_Error::Unknown);				break;
					}

					break;
				}
			}
			break;
		}
	}
}

void NetClient::Set_State(NetClient_State::Type state)
{
	m_state = state;
}

void NetClient::Set_Error(NetClient_Error::Type error)
{
	m_error = error;
	Set_State(NetClient_State::Error);
}

NetClient_State::Type NetClient::Get_State()
{
	return m_state;
}

NetClient_Error::Type NetClient::Get_Error()
{
	return m_error;
}

void NetClient::Set_Client_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods)
{
	m_missing_client_mods = mods;
}

void NetClient::Set_Server_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods)
{
	m_missing_server_mods = mods;
}

std::vector<NetPacket_EnforcedMod> NetClient::Get_Server_Missing_Mods()
{
	return m_missing_server_mods;
}

std::vector<NetPacket_EnforcedMod> NetClient::Get_Client_Missing_Mods()
{
	return m_missing_client_mods;
}

std::string NetClient::Get_Error_Message(NetClient_Error::Type error)
{
	switch (m_error)
	{
	case NetClient_Error::NONE:					return "";
	case NetClient_Error::Unknown:				return "#net_error_internal_error";
	case NetClient_Error::Unavailable:			return "#net_error_unavailable";
	case NetClient_Error::Timeout:				return "#net_error_timeout";

	case NetClient_Error::Wrong_Version:		return "#net_error_wrong_version";
	case NetClient_Error::Server_Full:			return "#net_error_server_full";
#ifdef OPT_PREMIUM_ACCOUNTS
	case NetClient_Error::Server_Premium:		return "#net_error_server_premium";
#endif
	case NetClient_Error::Banned:				return "#net_error_banned";
	case NetClient_Error::Kicked:				return "#net_error_kicked";
	case NetClient_Error::Lost_Connection:		return "#net_error_lost_connection";
	case NetClient_Error::Duplicate_Connection:	return "#net_error_duplicate_connection";
	case NetClient_Error::Transmission_Error:	return "#net_error_tranmission_error";
	case NetClient_Error::Server_Reserved:		return "#net_error_server_reserved";
	case NetClient_Error::Invalid_Ticket:		return "#net_error_invalid_ticket";

	case NetConnection_Error::Client_Mod_Missmatch:
	{
		std::string mods = "";
		for (std::vector<NetPacket_EnforcedMod>::iterator iter = m_missing_client_mods.begin(); iter != m_missing_client_mods.end(); iter++)
		{
			NetPacket_EnforcedMod& mod = *iter;
			if (mods != "")
			{
				mods += "\n";
			}
			mods += StringHelper::Format("%s (ID:%i)", mod.Workshop_Title.c_str(), mod.Workshop_ID);
		}
		return SF("#net_error_client_mod_missmatch", mods.c_str());
	}

	case NetConnection_Error::Server_Mod_Missmatch:
	{
		std::string mods = "";
		for (std::vector<NetPacket_EnforcedMod>::iterator iter = m_missing_server_mods.begin(); iter != m_missing_server_mods.end(); iter++)
		{
			NetPacket_EnforcedMod& mod = *iter;
			if (mods != "")
			{
				mods += "\n";
			}
			mods += StringHelper::Format("%s (ID:%i)", mod.Workshop_Title.c_str(), mod.Workshop_ID);
		}
		return SF("#net_error_server_mod_missmatch", mods.c_str());
	}

	default:									return "#net_error_internal_error";
	}
}

void NetClient::Connect_To_Server(OnlineMatching_Server server)
{
	DBG_ASSERT(server.User != NULL);

	DBG_LOG("Connecting to server '%s' (%s).", server.Name.c_str(), server.Address.To_String().c_str());

	m_server = server;

	if (!m_connection->Connect(server.User, false))
	{
		Set_Error(NetClient_Error::Unavailable);
	}
	else
	{
		Set_State(NetClient_State::Connecting);
	}
}

void NetClient::Disconnect_From_Server()
{
	DBG_LOG("Disconnecting from server.");

	m_connection->Disconnect();
}

void NetClient::On_Connected(NetConnection* connection)
{
	if (connection != m_connection)
	{
		return;
	}

	DBG_LOG("We are now connected to the server.");
}

void NetClient::On_Disconnected(NetConnection* connection)
{
	if (connection != m_connection)
	{
		return;
	}

	DBG_LOG("We have been disconencted from the server.");
}

NetConnection* NetClient::Get_Connection()
{
	return m_connection;
}

bool NetClient::Callback_RecievedUserListUpdate(NetPacket_S2C_UserListUpdate* packet)
{
	NetManager* manager = NetManager::Get();
	Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();
	NetConnection* connection = packet->Get_Recieved_From();

	std::vector<NetUser*> users = manager->Get_Net_Users();

	switch (packet->Update_Type)
	{
	case UserListUpdateType::Full:
		{
			// Remove all users that no longer exist.
			for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
			{
				NetUser* user = *iter;
				bool exists = false;
				
				for (std::vector<UserListState>::iterator i = packet->States.begin(); i != packet->States.end(); i++)
				{
					UserListState& state = *i;
					if (state.Net_ID == user->Get_Net_ID())
					{
						exists = true;
						break;
					}
				}

				if (exists == false)
				{
					DBG_LOG("User '%s' has left the game.", user->Get_Username().c_str());
					
					// If we are running local server we don't need to delete the user.
					if (NetManager::Get()->Server() == NULL)
					{
						On_User_Left(user);

						manager->Delete_Net_User(user);
						iter = users.begin();
					}
				}
			}

			// Add users that do not already exists.
			for (std::vector<UserListState>::iterator iter = packet->States.begin(); iter != packet->States.end(); iter++)
			{
				UserListState& state = *iter;
				NetUser* user = manager->Get_User_By_Net_ID(state.Net_ID);
				if (user == NULL)
				{
					DBG_LOG("User '%s' has joined the game.", state.Username.c_str());

					// If we are running local server we don't need to create the user.
					if (NetManager::Get()->Server() == NULL)
					{
						OnlineUser* online_user = platform->Get_User_By_SteamID(state.User_ID);
						if (online_user == NULL)
						{
							online_user = platform->Register_Remote_User(state.User_ID);
						}

						user = manager->Create_Net_User(NULL, state.Local_Index, online_user);
						user->Set_Username(state.Username.c_str());
						online_user->Set_Short_Country_Name(state.Short_Country_Name.c_str());
#ifdef OPT_PREMIUM_ACCOUNTS
						online_user->Set_Premium(state.Premium);
#endif
						user->Set_Net_ID(state.Net_ID);

						On_User_Joined(user);
					}
				}
			}
			break;
		}
	case UserListUpdateType::Join:
		{
			for (std::vector<UserListState>::iterator iter = packet->States.begin(); iter != packet->States.end(); iter++)
			{
				UserListState& state = *iter;
				NetUser* user = manager->Get_User_By_Net_ID(state.Net_ID);
				if (user == NULL)
				{
					DBG_LOG("User '%s' has joined the game.", state.Username.c_str());

					// If we are running local server we don't need to create the user.
					if (NetManager::Get()->Server() == NULL)
					{
						OnlineUser* online_user = platform->Get_User_By_SteamID(state.User_ID);
						if (online_user == NULL)
						{
							online_user = platform->Register_Remote_User(state.User_ID);
						}

						user = manager->Create_Net_User(NULL, state.Local_Index, online_user);
						online_user->Set_Short_Country_Name(state.Short_Country_Name.c_str());
#ifdef OPT_PREMIUM_ACCOUNTS
						online_user->Set_Premium(state.Premium);
#endif
						user->Set_Username(state.Username.c_str());
						user->Set_Net_ID(state.Net_ID);
					}
				}
			}
			break;
		}
	case UserListUpdateType::Leave:
		{
			for (std::vector<UserListState>::iterator iter = packet->States.begin(); iter != packet->States.end(); iter++)
			{
				UserListState& state = *iter;
				NetUser* user = manager->Get_User_By_Net_ID(state.Net_ID);
				if (user != NULL)
				{
					DBG_LOG("User '%s' has left the game.", user->Get_Username().c_str());
					
					// If we are running local server we don't need to delete the user.
					if (NetManager::Get()->Server() == NULL)
					{
						On_User_Left(user);

						manager->Delete_Net_User(user);
					}
				}
			}
			break;
		}
	}

	// Dump out all usernames for help.
	DBG_LOG("==== USER LIST ====");
	std::vector<NetUser*> net_users = NetManager::Get()->Get_Net_Users();
	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		NetUser* user = *iter;
#ifdef OPT_PREMIUM_ACCOUNTS
		DBG_LOG("\t[%i] %s (Premium=%i)", user->Get_Net_ID(), user->Get_Username().c_str(), user->Get_Online_User()->Get_Premium());
#else
		DBG_LOG("\t[%i] %s", user->Get_Net_ID(), user->Get_Username().c_str());
#endif
	}

	return true;
}

bool NetClient::Callback_RecievedNameChanged(NetPacket_C2S_NameChanged* packet)
{
	std::vector<NetUser*>& net_users = NetManager::Get()->Get_Net_Users();
	
	for (std::vector<NetUser*>::iterator iter = net_users.begin(); iter != net_users.end(); iter++)
	{
		NetUser* user = *iter;
		if (user->Get_Net_ID() == packet->Net_ID)
		{			
			DBG_LOG("Recieved name change for user '%s' -> '%s' (net id = %u).", user->Get_Username().c_str(), packet->Username.c_str(), (u32)user->Get_Net_ID());

			// Change username.
			user->Set_Username(packet->Username);

			break;
		}
	}

	return true;
}

bool NetClient::Callback_RecievedInitialState(NetPacket_S2C_InitialState* packet)
{
	// TODO: We don't actually transmit anything we care about here. I'm assuming we will in future lol!
	return true;
}

bool NetClient::Callback_RecievedChangeMap(NetPacket_S2C_ChangeMap* packet)
{
	// Only change map if we are not running a local server. If its local, map will already be flagged to change.
	if (NetManager::Get()->Server() == NULL)
	{
		NetManager::Get()->Change_Map_By_GUID(
			packet->Map_GUID.c_str(), 
			packet->Map_Workshop_ID, 
			packet->Map_Seed, 
			packet->Map_Load_Index,
			packet->Map_Dungeon_Level_Index);
	}
	return true;
}

bool NetClient::Callback_RecievedKick(NetPacket_S2C_Kick* packet)
{
	DBG_LOG("Recieved kick from server.");

	Set_Error(NetClient_Error::Kicked);

	return true;
}