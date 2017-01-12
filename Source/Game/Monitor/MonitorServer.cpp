// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Monitor/MonitorServer.h"
#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"
#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Monitor/Packets/GameStateMonitorPacket.h"

#include "Generic/ThirdParty/RapidJSON/include/rapidjson/document.h"
#include "Generic/ThirdParty/RapidJSON/include/rapidjson/writer.h"
#include "Generic/ThirdParty/RapidJSON/include/rapidjson/stringbuffer.h"

#ifdef PLATFORM_WIN32
#define GET_SOCKET_ERROR WSAGetLastError()
#else
#define GET_SOCKET_ERROR (errno)
#define SOCKET_ERROR -1
#endif

MonitorServerConnection::MonitorServerConnection()
	: m_socket(INVALID_SOCKET)
	, m_id(0)
	, m_reading_payload(false)
{
}

MonitorServerConnection::~MonitorServerConnection()
{
}

void MonitorServerConnection::Close()
{
	if (m_socket != INVALID_SOCKET)
	{
#ifdef PLATFORM_WIN32
		closesocket(m_socket);
#else
		close(m_socket);
#endif
		m_socket = INVALID_SOCKET;
	}
}

bool MonitorServerConnection::IsConnected()
{
	int error = 0;
#ifdef PLATFORM_WIN32
	int len = sizeof(error);
#else
	socklen_t len = sizeof(error);
#endif
	int retval = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
	return (retval == 0);
}

bool MonitorServerConnection::Poll()
{
	// We are connected!
	if (IsConnected())
	{
		u_long data_available = 0;

#ifdef PLATFORM_WIN32
		ioctlsocket(m_socket, FIONREAD, &data_available);
#else
		ioctl(m_socket, FIONREAD, &data_available);
#endif

		// Read in header.
		if (!m_reading_payload)
		{
			if (data_available >= 4)
			{
				m_payload.Reserve(4, false);
				int totalRead = recv(m_socket, m_payload.Buffer(), 4, 0);
				DBG_ASSERT(totalRead == 4);

				int* int_ptr = reinterpret_cast<int*>(m_payload.Buffer());
				m_payload_length = *int_ptr;
				m_payload_read = 0;
				m_payload.Reserve(m_payload_length + 1, false);
				m_payload.Buffer()[m_payload_length] = '\0';

				m_reading_payload = true;
			}
		}
		// Read in payload.
		else
		{
			int remaining = m_payload_length - m_payload_read;
			int toRead = Min(remaining, (int)data_available);
			if (toRead > 0)
			{
				int totalRead = recv(m_socket, m_payload.Buffer() + m_payload_read, toRead, 0);
				m_payload_read += totalRead;

				if (m_payload_read >= m_payload_length)
				{
					m_reading_payload = false;

					Recieve(m_payload.Buffer());
				}
			}
		}

		PollPeriodic();

		return true;
	}

	// Something done fucked up.
	else
	{
		return false;
	}
}

void MonitorServerConnection::PollPeriodic()
{
	GameStateMonitorPacket newState;
	newState.Username = OnlinePlatform::Get()->Get_Initial_User()->Get_Username();

	UIScene_Game* game = GameEngine::Get()->Get_UIManager()->Get_Scene_By_Type<UIScene_Game*>();
	if (game != NULL)
	{
		newState.Map = Game::Get()->Get_Map()->Get_Map_Header().Short_Name;

		if (NetManager::Get()->Server() != NULL)
		{
			newState.Host = StringHelper::Format("Hosting Game (%i Users)", NetManager::Get()->Get_Net_Users().size());
		}  
		else
		{
			newState.Host = StringHelper::Format("In Server: %s", GameNetManager::Get()->Get_Server_State().Host_Name.c_str());
		}
	}
	else
	{
		newState.Map = "In Menus";
		newState.Host = "";
	}

	if (m_game_state.Username != newState.Username ||
		m_game_state.Host != newState.Host ||
		m_game_state.Map != newState.Map)
	{
		m_game_state = newState;
		Send(&m_game_state);
	}
}

void MonitorServerConnection::Send(BaseMonitorPacket* Packet)
{
	MonitorPacketSerializer serializer;
	serializer.Writer.StartObject();
	Packet->Serialize(serializer, true);
	serializer.Writer.EndObject();

	const char* json = serializer.Buffer.GetString();

//	DBG_LOG("[Monitor] Send:\n%s\n\n", json);

	int data_sent = 0;
	int to_send = strlen(json);

	send(m_socket, (const char*)&to_send, 4, 0);

	while (data_sent < to_send)
	{
		int retval = send(m_socket, json + data_sent, to_send - data_sent, 0);
		if (retval < 0)
		{
			break;
		}
		data_sent += retval;
	}
}

void MonitorServerConnection::Recieve(const char* json)
{
	//DBG_LOG("[Monitor] Recv:\n%s\n\n", json);

	rapidjson::Document doc;
 	doc.Parse(json);

	MonitorPacketSerializer serializer;
	serializer.ReadRoot = &doc;

	BaseMonitorPacket* packet = BaseMonitorPacket::Create(doc["ClassName"].GetString());
	packet->Serialize(serializer, false);
	packet->Process(*this);
}

MonitorServer::MonitorServer()
	: m_socket(INVALID_SOCKET)
	, m_id_counter(0)
{
}

MonitorServer::~MonitorServer()
{
	for (std::vector<MonitorServerConnection>::iterator iter = m_connections.begin(); iter != m_connections.end(); iter++)
	{
		MonitorServerConnection& connection = *iter;
		connection.Close();
	}

	if (m_socket != INVALID_SOCKET)
	{
#ifdef PLATFORM_WIN32
		closesocket(m_socket);
#else
		close(m_socket);
#endif
		m_socket = INVALID_SOCKET;
	}
}

void MonitorServer::Init()
{
	DBG_LOG("Initializing monitor server.");

	// Create socket.
	DBG_ASSERT(m_socket == INVALID_SOCKET);
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	DBG_ASSERT_STR(m_socket != INVALID_SOCKET, "Failed to create socket with error 0x%08x.", GET_SOCKET_ERROR);

	// Dam nagle.
	int option_value = 1;
	setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option_value, sizeof(option_value));
	setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&option_value, sizeof(option_value));

	// Bind to local address.
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family			= AF_INET;
	addr.sin_port			= htons(*GameOptions::monitor_port);
#ifdef PLATFORM_WIN32
	addr.sin_addr.s_addr	= ADDR_ANY;
#else
	addr.sin_addr.s_addr	= INADDR_ANY;
#endif

	int ret = bind(m_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr));
	DBG_ASSERT_STR(ret != SOCKET_ERROR, "Failed to bind socket to listen address with error 0x%08x.", GET_SOCKET_ERROR);

	// Start listening for connections!
	ret = listen(m_socket, SOMAXCONN);
	DBG_ASSERT_STR(ret != SOCKET_ERROR, "Failed to listen on socket with error x%08x.", GET_SOCKET_ERROR);
}

void MonitorServer::Tick(const FrameTime& t)
{
	// Not init yet?
	if (m_socket == INVALID_SOCKET)
	{
		return;
	}

	// Any new connections?
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
			MonitorServerConnection connection;
			connection.m_id		= m_id_counter++;
			connection.m_socket = new_socket;
			
			struct sockaddr_in address = {0};     
#ifdef PLATFORM_WIN32  
			int addressLength = sizeof(address);   
#else
			socklen_t addressLength = sizeof(address);   
#endif
			
			getpeername(new_socket, (struct sockaddr*)&address, &addressLength);
			connection.m_remote_ip = inet_ntoa(address.sin_addr);       
			connection.m_remote_port = ntohs(address.sin_port);     

			DBG_LOG("[Monitor] Connection %i, connected from %s:%i.", 
				connection.m_id,
				connection.m_remote_ip.c_str(),
				connection.m_remote_port);

			m_connections.push_back(connection);
		}
	}

	// Poll sockets.
	for (std::vector<MonitorServerConnection>::iterator iter = m_connections.begin(); iter != m_connections.end(); iter++)
	{
		MonitorServerConnection& connection = *iter;
		if (!connection.Poll())
		{
			DBG_LOG("[Monitor] Connection %i, disconnected.", connection.m_id);
			m_connections.erase(iter);
			break;
		}
	}
}