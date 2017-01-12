// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/Steamworks/Steamworks_NetConnection.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/NetFileTransfer.h"
#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineServer.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"
#include "Engine/Online/OnlineClient.h"
#include "Engine/IO/BinaryStream.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Engine/EngineOptions.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/GameRunner.h"
#include "Engine/Resources/PackageFile.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include <algorithm>

#include "Generic/Helper/PersistentLogHelper.h"

//int g_auth_ticket_sessions = 0;

std::vector<Steamworks_CooldownEntry> Steamworks_NetConnection::g_cooldown_list;

void Steamworks_NetConnection::AddSteamIdToCooldownList(CSteamID id)
{
	if (IsSteamIdInCooldownList(id))
	{
		return;
	}

	static float COOLDOWN_DURATION = 2.0f;

	Steamworks_CooldownEntry cooldown;
	cooldown.Cooldown = COOLDOWN_DURATION;
	cooldown.Remote = id;
	g_cooldown_list.push_back(cooldown);

	DBG_LOG("Added user '%ull' to cooldown list.", id.ConvertToUint64());
}

bool Steamworks_NetConnection::IsSteamIdInCooldownList(CSteamID id)
{
	for (std::vector<Steamworks_CooldownEntry>::iterator iter = g_cooldown_list.begin(); iter != g_cooldown_list.end(); iter++)
	{
		if ((*iter).Remote == id)
		{
			return true;
		}
	}

	return false;
}

void Steamworks_NetConnection::PollCooldownList()
{
	float delta_t = GameEngine::Get()->Get_Time()->Get_Delta_Seconds();

	for (std::vector<Steamworks_CooldownEntry>::iterator iter = g_cooldown_list.begin(); iter != g_cooldown_list.end(); )
	{
		Steamworks_CooldownEntry& entry = *iter;		
		entry.Cooldown -= delta_t;
		
		if (entry.Cooldown <= 0.0f)
		{
			DBG_LOG("Removed user '%ull' from cooldown list.", entry.Remote.ConvertToUint64());
			iter = g_cooldown_list.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

Steamworks_NetConnection::Steamworks_NetConnection(Steamworks_NetConnection* parent)
	: NetConnection(parent)
	, m_is_loopback(false)
	, m_loopback_remote_connection(NULL)
	, m_auth_ticket(NULL)
	, m_in_auth_sessions(false)
	, m_callback_connection_failed			(this, &Steamworks_NetConnection::Callback_ConnectionFailed)
	, m_callback_connection_request			(this, &Steamworks_NetConnection::Callback_ConnectionRequest)
	, m_callback_connection_request_client	(this, &Steamworks_NetConnection::Callback_ConnectionRequestClient)
	, m_callback_ticket_auth_result			(this, &Steamworks_NetConnection::Callback_TicketAuthResult)
{
	DBG_LOG("Constructing new net connection 0x%08x.", this);
}

Steamworks_NetConnection::~Steamworks_NetConnection()
{	
	DBG_LOG("Destructing net connection 0x%08x.", this);

	if (m_loopback_remote_connection != NULL)
	{
		m_loopback_remote_connection->m_loopback_remote_connection = NULL;
		m_loopback_remote_connection = NULL;
	}

	// Try and send a disconnect gracefully if possible!
	if (m_state != NetConnection_State::Disconnecting && 
		m_state != NetConnection_State::Disconnected && 
		m_state != NetConnection_State::Error)
	{
		Disconnect();
	}

	// Make 100% sure everything is closed.
	if (m_is_loopback == false)
	{
		CloseP2PSession();
	}
}

void Steamworks_NetConnection::CloseP2PSession()
{
	NetManager* net_manager = NetManager::Get();
	if (net_manager->Server() != NULL && SteamGameServerNetworking())
	{
		SteamGameServerNetworking()->CloseP2PSessionWithUser(m_remote_user_id);
	}
	else if (SteamNetworking())
	{
		SteamNetworking()->CloseP2PSessionWithUser(m_remote_user_id);
	}

	DBG_LOG("Closed p2p session to  '%llu'.", m_remote_user_id.ConvertToUint64());
}

OnlineUser* Steamworks_NetConnection::Get_Online_User()
{
	Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();
	Steamworks_OnlineUser* user = platform->Get_User_By_SteamID(m_remote_user_id);
	
	if (user == NULL)
	{
		user = platform->Register_Remote_User(m_remote_user_id);
	}

	return user;
}

void Steamworks_NetConnection::Callback_ConnectionFailed(P2PSessionConnectFail_t* param)
{
	if (param->m_steamIDRemote != m_remote_user_id)
	{
		return;
	}

	DBG_LOG("Connection to user '%llu' failed with error %i.", param->m_steamIDRemote.ConvertToUint64(), param->m_eP2PSessionError);
	
	switch (param->m_eP2PSessionError)
	{
	case k_EP2PSessionErrorNotRunningApp:
		{
			Disconnect_Internal(NetConnection_Error::Unavailable, true);
			break;
		}
	case k_EP2PSessionErrorNoRightsToApp:
		{
			Disconnect_Internal(NetConnection_Error::Unavailable, true);
			break;
		}
	case k_EP2PSessionErrorDestinationNotLoggedIn:
		{
			Disconnect_Internal(NetConnection_Error::Unavailable, true);
			break;
		}
	case k_EP2PSessionErrorTimeout:
		{
			Disconnect_Internal(NetConnection_Error::Timeout, true);
			break;
		}
	default:
		{
			Disconnect_Internal(NetConnection_Error::Unknown, true);
			break;
		}
	}
}

void Steamworks_NetConnection::Callback_TicketAuthResult(ValidateAuthTicketResponse_t* param)
{
#ifndef OPT_DISABLE_TICKETING

	if (param->m_SteamID != m_remote_user_id)
	{
		return;
	}

	DBG_LOG("Got ticket authentication status (%i) for user '%llu'.", param->m_eAuthSessionResponse, param->m_SteamID.ConvertToUint64());
	switch (param->m_eAuthSessionResponse)
	{
		case k_EAuthSessionResponseOK:
			{
				DBG_LOG("User '%llu' is authenticated. Excellent, no shenanigans here!", param->m_SteamID.ConvertToUint64());
				break;
			}
		case k_EAuthSessionResponseUserNotConnectedToSteam:
			{
				DBG_LOG("User '%llu' is not connected to steam.", param->m_SteamID.ConvertToUint64());
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
		case k_EAuthSessionResponseNoLicenseOrExpired:
			{
				DBG_LOG("User '%llu' has no license for this app, or the license is expired.", param->m_SteamID.ConvertToUint64());
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
		case k_EAuthSessionResponsePublisherIssuedBan:
		case k_EAuthSessionResponseVACBanned:
			{
				DBG_LOG("User '%llu' is VAC banned.", param->m_SteamID.ConvertToUint64());

				if (NetManager::Get()->Server()->Is_Secure())
				{
					Disconnect(NetConnection_Error::Banned);
				}
				else
				{
					DBG_LOG("Ignoring VAC ban as we are running insecure.");
				}

				break;
			}
		case k_EAuthSessionResponseLoggedInElseWhere:
			{
				DBG_LOG("User '%llu' is logged in elsewhere.", param->m_SteamID.ConvertToUint64());
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
		case k_EAuthSessionResponseVACCheckTimedOut:
			{
				DBG_LOG("User '%llu' timed out when checking VAC status.", param->m_SteamID.ConvertToUint64());
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
		case k_EAuthSessionResponseAuthTicketCanceled:
			{
				DBG_LOG("User '%llu' cancelled auth ticket.", param->m_SteamID.ConvertToUint64());
				m_auth_ticket = NULL;
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
		case k_EAuthSessionResponseAuthTicketInvalidAlreadyUsed:
			{
				DBG_LOG("User '%llu' provided us an already used ticket.", param->m_SteamID.ConvertToUint64());
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
		case k_EAuthSessionResponseAuthTicketInvalid:
			{
				DBG_LOG("User '%llu' provided us an invalid ticket!", param->m_SteamID.ConvertToUint64());
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
		default:
			{
				DBG_LOG("User '%llu' failed authentication for an unknown reason.", param->m_SteamID.ConvertToUint64());
				Disconnect(NetConnection_Error::Invalid_Ticket);
				break;
			}
	}

#endif
}

void Steamworks_NetConnection::Callback_ConnectionRequest(P2PSessionRequest_t* param)
{
	DBG_LOG("User '%llu' (%i) is attempting to open communications with us.", param->m_steamIDRemote.ConvertToUint64(), param->m_steamIDRemote.GetUnAccountInstance());

	// If we are a client, we want to reject this. Only servers should be accepting p2p connections.
	if (SteamGameServer())
	{
		if (IsSteamIdInCooldownList(param->m_steamIDRemote))
		{
			DBG_LOG("Rejecting communication attempt as remote peer is in connection cooldown list (probably just disconnected).");
		}
		else
		{
			SteamGameServerNetworking()->AcceptP2PSessionWithUser(param->m_steamIDRemote);
		}
	}
	else
	{
		DBG_LOG("Rejecting communication attempt as running as client.");
	}
}

void Steamworks_NetConnection::Callback_ConnectionRequestClient(P2PSessionRequest_t* param)
{
	Callback_ConnectionRequest(param);
}

Steamworks_NetConnection* Steamworks_NetConnection::New_Connection_Detected(CSteamID id)
{
	// Only listening connections can recieve connection requests.
	if (m_listen_connection == false)
	{
		return NULL;
	}

	NetClient* net_client = NetManager::Get()->Client();
	NetServer* net_server = NetManager::Get()->Server();
	
	// Nope nope nope. To many connections,
	// someone is probably trying to DDoS us. Drop connections.
	if (NetManager::Get()->Get_Connection_Count() > *EngineOptions::net_max_active_connections)
	{
		DBG_LOG("User '%llu' is attempting to connect to us, but we exceeded the maximum number of concurrent connections. Dropping connection.", id.ConvertToUint64());
		return NULL;
	}

	DBG_LOG("User '%llu' is attempting to connect to us.", id.ConvertToUint64());

	// Make a new connection for the user.
	Steamworks_NetConnection* connection = new Steamworks_NetConnection(this);
	connection->m_listen_connection = false;
	connection->m_remote_user_id = id;
	connection->m_is_loopback = false;

#ifdef OPT_STEAM_ALLOW_DUPLICATE_CONNECTIONS
	bool loopback_exists = false;
	std::vector<NetConnection*>& connections = NetManager::Get()->Get_Connections();

	for (std::vector<NetConnection*>::iterator iter = connections.begin(); iter != connections.end(); iter++)
	{
		Steamworks_NetConnection* connection = static_cast<Steamworks_NetConnection*>(*iter);
		if (connection->m_is_loopback &&
			connection->m_loopback_remote_connection != NULL)
		{
			loopback_exists = true;
			break;
		}
	}

	if (!loopback_exists)
#endif
	{
		Steamworks_OnlineClient* online_client = static_cast<Steamworks_OnlineClient*>(Steamworks_OnlinePlatform::Get()->Client());
		if (online_client != NULL)
		{
			Steamworks_OnlineUser* user = static_cast<Steamworks_OnlineUser*>(Steamworks_OnlinePlatform::Get()->Get_Initial_User());
			if (user->Get_SteamID() == connection->m_remote_user_id)
			{
				connection->m_loopback_remote_connection = static_cast<Steamworks_NetConnection*>(net_client->Get_Connection());
				connection->m_loopback_remote_connection->m_loopback_remote_connection = connection;
				connection->m_is_loopback = true;
			}
		}
	}
	
	connection->Set_State(NetConnection_State::Connecting);
	m_children.push_back(connection);

	return connection;
}

bool Steamworks_NetConnection::Connect(OnlineUser* user, bool as_query_connection)
{
	Steamworks_OnlineUser* sw_user = static_cast<Steamworks_OnlineUser*>(user);
	CSteamID sid = sw_user->Get_SteamID();
	
	m_is_loopback = false;
	
	NetServer* net_server = NetManager::Get()->Server();
	Steamworks_OnlineServer* online_server = static_cast<Steamworks_OnlineServer*>(Steamworks_OnlinePlatform::Get()->Server());
	if (online_server != NULL)
	{
		Steamworks_OnlineUser* initial_user = static_cast<Steamworks_OnlineUser*>(OnlinePlatform::Get()->Get_Initial_User());

		if (online_server->Get_Server_SteamID() == sid || initial_user->Get_SteamID() == sid)
		{
			m_is_loopback = true;
		}
	}

	DBG_LOG("Creating P2P connection to steam-id '%llu' (loopback=%i).", sid.ConvertToUint64(), m_is_loopback);

	m_remote_user_id = sid;
	m_listen_connection = false;
	Set_State(NetConnection_State::Connecting);

	// Make a fake connection request to the server.
	if (m_is_loopback == true)
	{
		Steamworks_OnlineUser* local_user = static_cast<Steamworks_OnlineUser*>(Steamworks_OnlinePlatform::Get()->Get_Initial_User());
		
		Steamworks_NetConnection* listen_socket = static_cast<Steamworks_NetConnection*>(net_server->Get_Connection());

		listen_socket->New_Connection_Detected(local_user->Get_SteamID());
	}

	// Send initial handshake packet.
	DBG_LOG("Sending inital connection packet to '%llu'.", sid.ConvertToUint64());

	NetPacket_C2S_Connect packet;
	packet.Version = NetManager::Get()->Get_Net_Version();
#ifdef OPT_PREMIUM_ACCOUNTS
	packet.Premium_Account = false;;
#endif

	if (as_query_connection == true)
	{
		packet.Local_User_Count	= 0;
	}
	else
	{
		packet.Local_User_Count = OnlinePlatform::Get()->Get_Local_User_Count();
	}

	packet.Short_Country_Name = OnlinePlatform::Get()->Get_Initial_User()->Get_GeoIP_Result().CountryShortName;

	std::vector<PackageFile*> enforced_mods = GameEngine::Get()->Get_Runner()->Get_Server_Enforced_Packages();
	for (std::vector<PackageFile*> ::iterator iter2 = enforced_mods.begin(); iter2 != enforced_mods.end(); iter2++)
	{
		PackageFile* file = *iter2;

		NetPacket_EnforcedMod mod;
		mod.GUID = file->Get_Header()->guid;
		mod.Workshop_ID = file->Get_Header()->workshop_id;
		mod.Workshop_Title = file->Get_Header()->workshop_name;

		packet.Enforced_Mods.push_back(mod);
	}

	packet.Lobby_ID	= OnlineMatching::Get()->Get_Lobby_ID();
	
	for (int i = 0; i < OnlinePlatform::Get()->Get_Local_User_Count(); i++)
	{
		OnlineUser* local_user = OnlinePlatform::Get()->Get_Local_User_By_Index(i);
#ifdef OPT_PREMIUM_ACCOUNTS
		if (local_user->Get_Premium())
		{
			packet.Premium_Account = true;
			break;
		}
#endif
	}

	if (as_query_connection == false)
	{
		for (int i = 0; i < packet.Local_User_Count; i++)
		{
			OnlineUser* local_user = OnlinePlatform::Get()->Get_Local_User_By_Index(i);
			packet.Usernames[i] = local_user->Get_Username();
		}

#ifndef OPT_DISABLE_TICKETING
		// Get authentication ticket.
		DBG_ASSERT_STR(m_auth_ticket == NULL, "Old authentication ticket has not been disposed of! Thats not good! Shows broken authentication implementation! :(");
		m_auth_ticket = SteamUser()->GetAuthSessionTicket(packet.Auth_Ticket, MAX_AUTH_TICKET_LENGTH, &packet.Auth_Ticket_Length);
		DBG_ASSERT_STR(packet.Auth_Ticket_Length <= MAX_AUTH_TICKET_LENGTH, "Authentication ticket is to large for our packet buffer! Increase this shiz mofo.");
		DBG_LOG("Creating authentication ticket for '%llu'.", sid.ConvertToUint64());
#endif
	}

	Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool Steamworks_NetConnection::Listen(NetAddress local_address)
{
	DBG_LOG("Creating P2P listen connection for server.");
	
	m_listen_connection = true;
	Set_State(NetConnection_State::Listening);

	return true;
}

void Steamworks_NetConnection::Disconnect_Internal(NetConnection_Error::Type type, bool bSupressDisconnectedPacket)
{
	if (m_state == NetConnection_State::Disconnected)// ||
													 //m_state == NetConnection_State::Error)
	{
		return;
	}

	if (m_loopback_remote_connection != NULL)
	{
		m_loopback_remote_connection->m_loopback_remote_connection = NULL;
		m_loopback_remote_connection = NULL;
	}

	if (m_listen_connection == false)
	{
		if (m_parent == NULL)
		{
			if (!bSupressDisconnectedPacket && m_state == NetConnection_State::Connected)
			{
				// Politely tell the host we have disconnect.
				NetPacket_C2S_Disconnect packet;
				packet.Result = type;
				Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
			}

			// Cancel our authentication ticket.
			if (m_auth_ticket != NULL)
			{
				DBG_LOG("Cancelling authentication ticket.");

#ifndef OPT_DISABLE_TICKETING
				SteamUser()->CancelAuthTicket(m_auth_ticket);
#endif
				m_auth_ticket = NULL;
			}
		}

		Set_State(NetConnection_State::Disconnected);

		if (type != NetConnection_Error::NONE)
		{
			Set_Error(type);
		}
		
		// Server steamworks connection.
		if (m_is_loopback == false)
		{
			if (m_parent == NULL)
			{
				CloseP2PSession();
			}

#ifndef OPT_DISABLE_TICKETING
			// Only end auth session if not a query connection.
			if (m_connect_packet.Local_User_Count != 0)
			{
				if (SteamGameServer())
				{
					if (m_in_auth_sessions)
					{
						SteamGameServer()->EndAuthSession(m_remote_user_id);
						m_in_auth_sessions = false;
						//g_auth_ticket_sessions--;
					}
				}
			}
#endif
		}

		// Connections are ususally disposed of by their NetUser's, if this connection
		// dosen't have any users (ie. it's a query connection), then dispose it ourselves.
		if (Is_Dead() == false && m_has_users == false && m_parent != NULL)
		{
			Dispose();
		}
	}

	if (m_parent != NULL)
	{
		AddSteamIdToCooldownList(m_remote_user_id);
	}
}

void Steamworks_NetConnection::Disconnect(NetConnection_Error::Type type)
{	
	Disconnect_Internal(type);
}

void Steamworks_NetConnection::Send(NetPacket* packet, int channel, bool reliable)
{
	SendTo(packet, channel, reliable, m_remote_user_id);
}

void Steamworks_NetConnection::SendTo(NetPacket* packet, int channel, bool reliable, OnlineUser* user)
{
	Steamworks_OnlineUser* sw = static_cast<Steamworks_OnlineUser*>(user);
	SendTo(packet, channel, reliable, sw->Get_SteamID());
}

void Steamworks_NetConnection::SendTo(NetPacket* packet, int channel, bool reliable, CSteamID id)
{
	DBG_ASSERT_STR(m_listen_connection == false, "Cannot send to a listen socket!");
	DBG_ASSERT_STR(channel >= 0 && channel < MAX_NET_CONNECTION_CHANNELS, "Invalid connection channel.");

	// Don't try and send if we've disconnected.
	if (m_state != NetConnection_State::Connected &&
		m_state != NetConnection_State::Connecting)
	{
		return;
	}

	static const int max_packet_size = 1024; // Steam max-packet size.

	BinaryStream data_stream;
	data_stream.Reserve_Exactly(max_packet_size);
	packet->Serialize(&data_stream);

	DBG_ASSERT_STR(data_stream.Length() <= max_packet_size || reliable == true, "Unreliable packet exceeds steamworks MTU.");

	EP2PSend send_type = k_EP2PSendUnreliable;
	if (reliable == true)
	{
		send_type = k_EP2PSendReliable;
	}
	
	// Break packet into fragments if larger than MTU.
	bool packets_sent = 0;
	for (unsigned int frag_offset = 0; frag_offset < data_stream.Length() || packets_sent == 0; frag_offset += max_packet_size)
	{
		unsigned int frag_size = Min(max_packet_size, data_stream.Length() - frag_offset);
		bool is_last_frag = (frag_offset + frag_size) >= data_stream.Length();

		BinaryStream frag_stream;
		frag_stream.Reserve_Exactly(max_packet_size);
		frag_stream.Write<unsigned int>(packet->Get_Type_ID());
		frag_stream.Write<unsigned char>(is_last_frag ? 1 : 0);

		if (frag_size > 0)
		{
			frag_stream.WriteBuffer(data_stream.Data(), frag_offset, frag_size);
		}

		packets_sent++;

		//DBG_LOG("[Connection %i] >> %s (%i bytes)", this, packet->Get_Name(), frag_stream.Length());

		OnPacketOut(packet->Get_Type_ID(), frag_stream.Length());

		if (IsSimulatingLatency())
		{
			float latency = *EngineOptions::net_simulate_latency;
			float jitter  = *EngineOptions::net_simulate_jitter;

			Steamworks_PendingPacket pending;
			pending.SendTime  = Platform::Get()->Get_Ticks() + latency + Random::Static_Next_Double(0.0f, jitter);
			pending.SendType = send_type;
			pending.Destination = id;
			pending.Channel = channel;
			pending.Buffer.Set(frag_stream.Data(), frag_stream.Length());

			m_pending_send_packets.push_back(pending);
		}
		else
		{
			SendPacket(reinterpret_cast<const void*>(frag_stream.Data()), frag_stream.Length(), send_type, channel, id);
		}
	}
}

bool Steamworks_NetConnection::IsSimulatingLatency()
{
	return (*EngineOptions::net_simulate_latency != 0) || (*EngineOptions::net_simulate_jitter != 0);
}

void Steamworks_NetConnection::PollPendingPackets()
{
	if (IsSimulatingLatency())
	{
		double time = Platform::Get()->Get_Ticks();

		for (std::vector<Steamworks_PendingPacket>::iterator iter = m_pending_send_packets.begin(); iter != m_pending_send_packets.end(); )
		{
			Steamworks_PendingPacket& packet = *iter;
			if (time >= packet.SendTime)
			{
				SendPacket(packet.Buffer.Buffer(), packet.Buffer.Size(), packet.SendType, packet.Channel, packet.Destination);
				iter = m_pending_send_packets.erase(iter);
			}
			else
			{
				// As soon as we get to a packet we cannot send we abort. We don't want to send packets out of order.
				break;
			}
		}

		for (std::vector<Steamworks_PendingPacket>::iterator iter = m_pending_recv_packets.begin(); iter != m_pending_recv_packets.end(); )
		{
			Steamworks_PendingPacket& packet = *iter;
			if (time >= packet.SendTime)
			{
				RecievePacket(packet.Buffer.Buffer(), packet.Buffer.Size(), packet.Source, packet.Channel);
				iter = m_pending_recv_packets.erase(iter);
			}
			else
			{
				// As soon as we get to a packet we cannot send we abort. We don't want to send packets out of order.
				break;
			}
		}
	}
}

void Steamworks_NetConnection::SendPacket(const void* data, int data_length, EP2PSend send_type, int channel, CSteamID id)
{
	if (m_is_loopback == true && id == m_remote_user_id)
	{
		if (m_loopback_remote_connection != NULL)
		{
			NetPacket* new_packet = m_loopback_remote_connection->Parse_Net_Packet(reinterpret_cast<const char*>(data), data_length);
			if (new_packet != NULL)
			{
				m_loopback_remote_connection->Handle_Packet(new_packet);
			}
		}
	}
	else
	{
		if (m_parent != NULL)
		{
			SteamGameServerNetworking()->SendP2PPacket(id, data, data_length, send_type, channel);
		}
		else
		{
			SteamNetworking()->SendP2PPacket(id, data, data_length, send_type, channel);
		}
	}
}

void Steamworks_NetConnection::Poll()
{	
	//DBG_LOG("Auth Sessions: %i", g_auth_ticket_sessions)

	// Grab the correct steam network depending if we are server side or not.
	ISteamNetworking* network = NULL;
	bool server = false;
	if (m_listen_connection == true || m_parent != NULL)
	{
		// This is a poor hack, we check the server is valid before proceeding.
		// If we don't, we can end up trying to update game servers after disconnect.
		if (NetManager::Get()->Server() == NULL)
		{
			return;
		}

		network = SteamGameServerNetworking();
		server = true;
	}
	else
	{
		network = SteamNetworking();
		server = false;
	}

	// Platform not setup yet? :(
	if (network == NULL)
	{
		return;
	}

	// Poll pending packets.
	PollPendingPackets();

	// We don't care about child-connections, we will route the messages
	// to them from the listen socket.
	if (m_parent == NULL && m_is_loopback == false)
	{
		for (int channel = 0; channel < MAX_NET_CONNECTION_CHANNELS; channel++)
		{
			unsigned int data_available = 0;
			CSteamID remote_user;

			if (network->IsP2PPacketAvailable(&data_available, channel))
			{
				m_read_buffer.Reserve(data_available, false);

				if (network->ReadP2PPacket(m_read_buffer.Buffer(), data_available, &data_available, &remote_user, channel))
				{
					if (IsSimulatingLatency())
					{
						float latency = *EngineOptions::net_simulate_latency;
						float jitter  = *EngineOptions::net_simulate_jitter;

						Steamworks_PendingPacket pending;
						pending.SendTime  = Platform::Get()->Get_Ticks() + latency + Random::Static_Next_Double(0.0f, jitter);
						pending.Source = remote_user;
						pending.Channel = channel;
						pending.Buffer.Set(m_read_buffer.Buffer(), m_read_buffer.Size());

						m_pending_recv_packets.push_back(pending);
					}
					else
					{
						RecievePacket(m_read_buffer.Buffer(), data_available, remote_user, channel);
					}
				}
			}
		}
	}

	// Poll base stuff.
	NetConnection::Poll();
}

void Steamworks_NetConnection::PollGlobals()
{
	PollCooldownList();
}

void Steamworks_NetConnection::PollMainThread()
{
	// Grab the correct steam network depending if we are server side or not.
	ISteamNetworking* network = NULL;
	bool server = false;
	if (m_listen_connection == true || m_parent != NULL)
	{
		// This is a poor hack, we check the server is valid before proceeding.
		// If we don't, we can end up trying to update game servers after disconnect.
		if (NetManager::Get()->Server() == NULL)
		{
			return;
		}

		network = SteamGameServerNetworking();
		server = true;
	}
	else
	{
		network = SteamNetworking();
		server = false;
	}

	// Platform not setup yet? :(
	if (network == NULL)
	{
		return;
	}

	// If we are not a listen connection, then disconnect if connection state changes.
	if (!m_listen_connection &&
		m_state == NetConnection_State::Connecting &&
		m_state == NetConnection_State::Connected)
	{
		P2PSessionState_t sessionState;
		network->GetP2PSessionState(m_remote_user_id, &sessionState);

		if (!sessionState.m_bConnecting && !sessionState.m_bConnectionActive)
		{
			DBG_LOG("Connection 0x%08x has disconnected due to p2p session being lost.", this);
			Disconnect_Internal(NetConnection_Error::Lost_Connection, true);
			return;
		}
	}

	// Poll base stuff.
	NetConnection::PollMainThread();
}

void Steamworks_NetConnection::RecievePacket(const void* buffer, int bufferLength, CSteamID source, int channel)
{
	// Grab the correct steam network depending if we are server side or not.
	ISteamNetworking* network = NULL;
	bool server = false;
	if (m_listen_connection == true || m_parent != NULL)
	{
		// This is a poor hack, we check the server is valid before proceeding.
		// If we don't, we can end up trying to update game servers after disconnect.
		if (NetManager::Get()->Server() == NULL)
		{
			return;
		}

		network = SteamGameServerNetworking();
		server = true;
	}
	else
	{
		network = SteamNetworking();
		server = false;
	}

	// Platform not setup yet? :(
	if (network == NULL)
	{
		return;
	}

	// Route packet to correct net connection.
	if (server == true)
	{
		Steamworks_NetConnection* remote_conn = NULL;

		for (std::vector<NetConnection*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			Steamworks_NetConnection* connection = static_cast<Steamworks_NetConnection*>(*iter);
			if (connection->m_remote_user_id == source)
			{
				remote_conn = connection;
				break;
			}
		}

		// New user?
		if (remote_conn == NULL)
		{
			remote_conn = New_Connection_Detected(source);
		}

		// Parse packet.					
		NetPacket* packet = remote_conn->Parse_Net_Packet(reinterpret_cast<const char*>(buffer), bufferLength);
		if (packet != NULL)
		{
			OnPacketIn(packet->Get_Type_ID(), bufferLength);

			//DBG_LOG("[Connection %i] << %s (%i bytes)", this, packet->Get_Name(), bufferLength);

			remote_conn->Handle_Packet(packet);
		}
	}
	else
	{							
		NetPacket* packet = Parse_Net_Packet(reinterpret_cast<const char*>(buffer), bufferLength);
		if (packet != NULL)
		{
			OnPacketIn(packet->Get_Type_ID(), bufferLength);

			//DBG_LOG("[Connection %i] << %s (%i bytes)", this, packet->Get_Name(), bufferLength);

			// If this is a lobby packet it could potentially come from anyone so just suck it
			// up and pass it on.
			if (packet->Is_OutOfBand())
			{
				Queue_Packet(packet);
			}

			// Is this not our end-user?
			else if (m_remote_user_id != source)
			{
				// Might be another connection that needs this packet (query connection etc).
				bool found_user = false;

				std::vector<NetConnection*>& connections = NetManager::Get()->Get_Connections();

				for (std::vector<NetConnection*>::iterator iter = connections.begin(); iter != connections.end(); iter++)
				{
					Steamworks_NetConnection* connection = static_cast<Steamworks_NetConnection*>(*iter);
					if (connection->m_remote_user_id == source)
					{
						NetPacket* parsed_packet = connection->Parse_Net_Packet(reinterpret_cast<const char*>(buffer), bufferLength);
						if (parsed_packet != NULL)
						{
							connection->Handle_Packet(parsed_packet);
						}

						found_user = true;
						break;
					}
				}

				if (found_user == false)
				{
					DBG_LOG("Ignoring packet from unknown peer '%llu'.", source.ConvertToUint64());
				}
			}

			// Normal statefull packet, process as normal.
			else
			{
				Handle_Packet(packet);
			}
		}
	}
}

NetConnection_Error::Type Steamworks_NetConnection::Validate_New_Connection(NetPacket_C2S_Connect* packet)
{
	// Don't do ticket auth if we are a query connection (denoted by no users).
	if (packet->Local_User_Count == 0)
	{
		return NetConnection_Error::NONE;
	}

	// If in offline mode, we don't need to deal with auth sessions.
	if (!NetManager::Get()->Is_Dedicated_Server())
	{
		if (!OnlinePlatform::Get()->Is_Online())
		{
			return NetConnection_Error::NONE;
		}
	}

	// Do an authentication sanity check.
#ifndef OPT_DISABLE_TICKETING
	EBeginAuthSessionResult auth_basic_result = SteamGameServer()->BeginAuthSession(packet->Auth_Ticket, packet->Auth_Ticket_Length, m_remote_user_id);
	m_in_auth_sessions = true;

	//g_auth_ticket_sessions++;
	switch (auth_basic_result)
	{
	case k_EBeginAuthSessionResultOK:
		{
			DBG_LOG("Ticket from connection from '%llu' was valid.", m_remote_user_id.ConvertToUint64());
			return NetConnection_Error::NONE;
		}
	case k_EBeginAuthSessionResultInvalidTicket:			
		{
			DBG_LOG("Ticket from connection from '%llu' was invalid, rejecting connection.", m_remote_user_id.ConvertToUint64());
			return NetConnection_Error::Invalid_Ticket;
		}
#ifndef OPT_STEAM_ALLOW_DUPLICATE_CONNECTIONS
	case k_EBeginAuthSessionResultDuplicateRequest:
		{
			DBG_LOG("Ticket from connection from '%llu' was a duplicate, rejecting connection.", m_remote_user_id.ConvertToUint64());
			return NetConnection_Error::Invalid_Ticket;
		}
#endif
	case k_EBeginAuthSessionResultInvalidVersion:
		{
			DBG_LOG("Ticket from connection from '%llu' was an invalid version, rejecting connection.", m_remote_user_id.ConvertToUint64());
			return NetConnection_Error::Invalid_Ticket;
		}
	case k_EBeginAuthSessionResultGameMismatch:
		{
			DBG_LOG("Ticket from connection from '%llu' was for another game, rejecting connection.", m_remote_user_id.ConvertToUint64());
			return NetConnection_Error::Invalid_Ticket;
		}
	case k_EBeginAuthSessionResultExpiredTicket:
		{
			DBG_LOG("Ticket from connection from '%llu' was expired, rejecting connection.", m_remote_user_id.ConvertToUint64());
			return NetConnection_Error::Invalid_Ticket;
		}
	default:
		{
			DBG_LOG("Ticket from connection from '%llu' was invalid for an unknown reason, rejecting connection.", m_remote_user_id.ConvertToUint64());
			return NetConnection_Error::Invalid_Ticket;
		}
	}
#endif

	return NetConnection_Error::NONE;
}

bool Steamworks_NetConnection::Has_Same_Endpoint_As(NetConnection* other)
{
	Steamworks_NetConnection* other_real = static_cast<Steamworks_NetConnection*>(other);
	return (other_real->m_remote_user_id == m_remote_user_id);
}

