// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/Network/BSD/BSD_NetConnection.h"
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

#include <algorithm>

#ifdef PLATFORM_WIN32
#define GET_SOCKET_ERROR WSAGetLastError()
#else
#define GET_SOCKET_ERROR (SOCKET_ERROR)
#endif

BSD_NetConnection::BSD_NetConnection(BSD_NetConnection* parent)
	: NetConnection(parent)
	, m_socket(INVALID_SOCKET)
{
	DBG_LOG("Constructing new net connection 0x%08x.", this);
}

BSD_NetConnection::~BSD_NetConnection()
{	
	DBG_LOG("Destructing net connection 0x%08x.", this);

	// Try and send a disconnect gracefully if possible!
	Disconnect();

	// Destroy socket!
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

OnlineUser* BSD_NetConnection::Get_Online_User()
{
	Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();
	Steamworks_OnlineUser* user = platform->Get_User_By_SteamID(m_remote_user_id);

	if (user == NULL)
	{
		user = platform->Register_Remote_User(m_remote_user_id);
	}

	return user;
}

bool BSD_NetConnection::Connect(OnlineUser* user, bool as_query_connection)
{
	Steamworks_OnlineUser* sw_user = static_cast<Steamworks_OnlineUser*>(user);
	CSteamID sid = sw_user->Get_SteamID();

	DBG_LOG("Creating P2P connection to steam-id '%llu'.", sid.ConvertToUint64());

	// Create socket.
	DBG_ASSERT(m_socket == INVALID_SOCKET);
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	DBG_ASSERT_STR(m_socket != INVALID_SOCKET, "Failed to create socket with error 0x%08x.", GET_SOCKET_ERROR);

	int option_value = 1;
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&option_value, sizeof(option_value));

	// Connect to local address.
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(BSD_SOCKET_LISTEN_PORT);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = connect(m_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	if (ret == SOCKET_ERROR)
	{
		DBG_LOG("Failed to connect with error 0x%08x.", GET_SOCKET_ERROR);	
		return false;
	}

	{
		// Send our steam-id.
		u64 our_steamid = SteamUser()->GetSteamID().ConvertToUint64(); 
		send(m_socket, (const char*)&our_steamid, sizeof(u64), 0);
		//DBG_ASSERT(ret < sizeof(u64), "Failed to send steam id with error 0x%08x.", GET_SOCKET_ERROR);	
	}

	{
		// Recieve id assigned to us.
		u64 our_steamid = 0;
		recv(m_socket, (char*)&our_steamid, sizeof(u64), 0);
		CSteamID other_id(our_steamid);
	
		Steamworks_OnlineUser* local_user = static_cast<Steamworks_OnlineUser*>(OnlinePlatform::Get()->Get_Local_User_By_Index(0));
		local_user->Set_SteamID(our_steamid);
	}

	m_remote_user_id = sid;
	m_listen_connection = false;
	Set_State(NetConnection_State::Connecting);

	// Send initial handshake packet.
	DBG_LOG("Sending inital connection packet to '%llu'.", sid.ConvertToUint64());

	NetPacket_C2S_Connect packet;
	packet.Version = NetManager::Get()->Get_Net_Version();
#ifdef OPT_PREMIUM_ACCOUNTS
	packet.Premium_Account = false;
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

	if (as_query_connection == false)
	{
		for (int i = 0; i < packet.Local_User_Count; i++)
		{
			OnlineUser* local_user = OnlinePlatform::Get()->Get_Local_User_By_Index(i);
#ifdef OPT_PREMIUM_ACCOUNTS
			if (local_user->Get_Premium())
			{
				packet.Premium_Account = true;
			}
#endif
			packet.Usernames[i] = local_user->Get_Username();
		}
	}

	Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	return true;
}

bool BSD_NetConnection::Listen(NetAddress local_address)
{
	DBG_LOG("Creating P2P listen connection for server.");

	// Create socket.
	DBG_ASSERT(m_socket == INVALID_SOCKET);
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	DBG_ASSERT_STR(m_socket != INVALID_SOCKET, "Failed to create socket with error 0x%08x.", GET_SOCKET_ERROR);
	
	int option_value = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option_value, sizeof(option_value));
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&option_value, sizeof(option_value));

	// Bind to local port.
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(BSD_SOCKET_LISTEN_PORT);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int ret = bind(m_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	DBG_ASSERT_STR(ret != SOCKET_ERROR, "Failed to bind socket to listen address with error 0x%08x.", GET_SOCKET_ERROR);

	// Start listening for connections!
	ret = listen(m_socket, SOMAXCONN);
	DBG_ASSERT_STR(ret != SOCKET_ERROR, "Failed to listen on socket with error x%08x.", GET_SOCKET_ERROR);
	
	m_listen_connection = true;
	Set_State(NetConnection_State::Listening);

	return true;
}

void BSD_NetConnection::Disconnect(NetConnection_Error::Type type)
{	
	if (m_state == NetConnection_State::Disconnected)// ||
		//m_state == NetConnection_State::Error)
	{
		return;
	}

	if (m_listen_connection == false)
	{
		if (m_parent == NULL)
		{
			// Politely tell the host we have disconnect.
			NetPacket_C2S_Disconnect packet;
			packet.Result = type;
			Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
		}

		if (m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;		
		}

		Set_State(NetConnection_State::Disconnected);	

		if (type != NetConnection_Error::NONE)
		{
			Set_Error(type);
		}

		// Connections are ususally disposed of by their NetUser's, if this connection
		// dosen't have any users (ie. it's a query connection), then dispose it ourselves.
		if (Is_Dead() == false && m_has_users == false && m_parent != NULL)
		{
			Dispose();
		}
	}
}

void BSD_NetConnection::Send(NetPacket* packet, int channel, bool reliable)
{
	BinaryStream data_stream;
	packet->Serialize(&data_stream);

	BinaryStream frag_stream;
	frag_stream.Write<unsigned int>(packet->Get_Type_ID());
	frag_stream.Write<unsigned int>(data_stream.Length());
	frag_stream.WriteBuffer(data_stream.Data(), 0, data_stream.Length());

	OnPacketOut(packet->Get_Type_ID(), frag_stream.Length());

	int ret = send(m_socket, frag_stream.Data(), frag_stream.Length(), 0);
//	DBG_ASSERT_STR(ret == frag_stream.Length(), "Failed to send on socket with error x%08x.", GET_SOCKET_ERROR);
}

void BSD_NetConnection::SendTo(NetPacket* packet, int channel, bool reliable, OnlineUser* user)
{
	// Send-To is not supported on BSD connections. These are only used for lobby packets anyhow.
	DBG_LOG("SendTo attempted on BSD socket - BSD sockets do not support this (yet!).");
}

void BSD_NetConnection::Poll()
{	
	// Accept new connections.
	if (m_listen_connection)
	{
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		fd_set set;
		FD_ZERO(&set); 
		FD_SET(m_socket, &set); 
		
		// Sockets pending?
		int ret = select(m_socket + 1, &set, NULL, NULL, &timeout);
		if (ret == 1)
		{
			// Accept new connections.
			SOCKET new_socket = accept(m_socket, NULL, NULL);
			if (new_socket != INVALID_SOCKET) 
			{
				// Recieve steamid.
				u64 other_steamid = 0;
				recv(new_socket, (char*)&other_steamid, sizeof(u64), 0);
				CSteamID other_id(other_steamid);

#ifdef OPT_STEAM_ALLOW_DUPLICATE_CONNECTIONS
				// If we allow duplicate connections we have to bodge the steamid if user already exists.
				Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();
				while (platform->Get_User_By_SteamID(other_id) != NULL)
				{
					other_steamid += 1;
					other_id = CSteamID(other_steamid);
				}
#endif

				// send new id back to the user.
				send(new_socket, (const char*)&other_steamid, sizeof(u64), 0);

				New_Connection_Detected(other_id, new_socket);
			}
		}
	}

	// Poll for new data.
	else
	{
		u_long data_available = 0;

#ifdef PLATFORM_WIN32
		ioctlsocket(m_socket, FIONREAD, &data_available);
#else
		ioctl(m_socket, FIONREAD, &data_available);
#endif

		if (data_available > 0)
		{
			int old_size = m_read_buffer.Size();
			m_read_buffer.Reserve(old_size + data_available, true);

			int ret = recv(m_socket, m_read_buffer.Buffer() + old_size, data_available, 0);
			DBG_ASSERT_STR(ret == data_available, "Failed to read on socket with error x%08x.", GET_SOCKET_ERROR);

			// Parse any packets we can construct from the data.
			static int header_size = sizeof(unsigned int) + sizeof(unsigned int);
			while (m_read_buffer.Size() > header_size)
			{
				unsigned int type_id = *reinterpret_cast<unsigned int*>(m_read_buffer.Buffer());
				unsigned int payload_size = *reinterpret_cast<unsigned int*>(m_read_buffer.Buffer() + sizeof(unsigned int));

				if ((unsigned int)m_read_buffer.Size() >= header_size + payload_size)
				{
					OnPacketIn(type_id, header_size + payload_size);

					int packet_size = (sizeof(unsigned int) + sizeof(unsigned int) + payload_size);

					// Read in the payload.
					DataBuffer payload(m_read_buffer.Buffer(), packet_size);

					NetPacket* packet = Parse_Net_Packet(payload.Buffer(), payload.Size(), true);
					if (packet != NULL)
					{
						// If this is a lobby packet it could potentially come from anyone so just suck it
						// up and pass it on.
						if (packet->Is_OutOfBand())
						{
							Queue_Packet(packet);
						}

						// Normal statefull packet, process as normal.
						else
						{
							Handle_Packet(packet);
						}
					}

					// Remove from read buffer.
					DataBuffer final;
					final.Reserve(m_read_buffer.Size() - packet_size);
					memcpy(final.Buffer(), m_read_buffer.Buffer() + packet_size, m_read_buffer.Size() - packet_size);
					m_read_buffer = final;
				}
				else
				{
					break;
				}
			}
		}
	}

	// Poll base stuff.
	NetConnection::Poll();
}

NetConnection_Error::Type BSD_NetConnection::Validate_New_Connection(NetPacket_C2S_Connect* packet)
{
	return NetConnection_Error::NONE;
}

BSD_NetConnection* BSD_NetConnection::New_Connection_Detected(CSteamID id, SOCKET socket)
{
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
	BSD_NetConnection* connection = new BSD_NetConnection(this);
	connection->m_listen_connection = false;
	connection->m_remote_user_id = id;
	connection->m_socket = socket;

	connection->Set_State(NetConnection_State::Connecting);
	m_children.push_back(connection);

	return connection;
}

bool BSD_NetConnection::Has_Same_Endpoint_As(NetConnection* other)
{
	BSD_NetConnection* other_real = static_cast<BSD_NetConnection*>(other);
	return (other_real->m_remote_user_id == m_remote_user_id);
}

#endif