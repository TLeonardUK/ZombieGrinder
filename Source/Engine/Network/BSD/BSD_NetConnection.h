// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_NETWORK_BSD_NET_CONNECTION_
#define _ENGINE_NETWORK_BSD_NET_CONNECTION_

#include "Engine/Network/NetConnection.h"
#include "Engine/Network/Packets/EnginePackets.h"

#include "Generic/Types/DataBuffer.h"

#include "Generic/Types/CircleBuffer.h"

#include "Engine/Network/Packets/PacketDispatcher.h"

#ifdef PLATFORM_WIN32
#include <winsock2.h>
#endif

// WARNING WARNING WARNING
// WARNING WARNING WARNING
// WARNING WARNING WARNING

// BSD SOCKETS ARE NOT SHIPPABLE! These are used for local testing with multiple users only, nothing is async
// and will block! If you want to use BSD sockets, lots of work is required!

// WARNING WARNING WARNING
// WARNING WARNING WARNING
// WARNING WARNING WARNING

#define BSD_SOCKET_LISTEN_PORT 12316

class Mutex;
class NetPacket_C2S_Connect;

class BSD_NetConnection : public NetConnection
{
	MEMORY_ALLOCATOR(BSD_NetConnection, "Network");

private:		
	DataBuffer								m_read_buffer;
	CSteamID								m_remote_user_id;

	SOCKET									m_socket;

protected:
	void Poll();

	BSD_NetConnection* New_Connection_Detected(CSteamID id, SOCKET socket);

public:
	BSD_NetConnection(BSD_NetConnection* parent = NULL);
	~BSD_NetConnection();

	NetConnection_Error::Type Validate_New_Connection(NetPacket_C2S_Connect* packet);

	OnlineUser* Get_Online_User();

	bool Has_Same_Endpoint_As(NetConnection* other);

	bool Connect(OnlineUser* user, bool as_query_connection = false);
	bool Listen(NetAddress local_address);
	void Disconnect(NetConnection_Error::Type type = NetConnection_Error::NONE);
	void Send(NetPacket* packet, int channel = 0, bool reliable = false);
	void SendTo(NetPacket* packet, int channel, bool reliable, OnlineUser* user);

};

#endif

#endif