// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NETWORK_STEAMWORKS_NET_CONNECTION_
#define _ENGINE_NETWORK_STEAMWORKS_NET_CONNECTION_

#include "Engine/Network/NetConnection.h"
#include "Engine/Network/Packets/EnginePackets.h"

#include "Generic/Types/DataBuffer.h"

#include "public/steam/steam_api.h"
#include "public/steam/steam_gameserver.h"

#include "Generic/Types/CircleBuffer.h"

#include "Engine/Network/Packets/PacketDispatcher.h"

class Mutex;
class NetPacket_C2S_Connect;

struct Steamworks_PendingPacket
{
	double		SendTime;
	EP2PSend	SendType;
	CSteamID	Source;
	CSteamID	Destination;
	int			Channel;
	DataBuffer	Buffer;
};

struct Steamworks_CooldownEntry
{
	CSteamID	Remote;
	float		Cooldown;
};

class Steamworks_NetConnection : public NetConnection
{
	MEMORY_ALLOCATOR(Steamworks_NetConnection, "Network");

private:		
	DataBuffer								m_read_buffer;

	CSteamID								m_remote_user_id;

	Steamworks_NetConnection*				m_loopback_remote_connection;
	bool									m_is_loopback;

	HAuthTicket								m_auth_ticket;
	bool									m_in_auth_sessions;

	std::vector<Steamworks_PendingPacket>	m_pending_send_packets;
	std::vector<Steamworks_PendingPacket>	m_pending_recv_packets;
		
	STEAM_CALLBACK							(Steamworks_NetConnection, Callback_ConnectionFailed,			P2PSessionConnectFail_t,		m_callback_connection_failed);
	STEAM_GAMESERVER_CALLBACK				(Steamworks_NetConnection, Callback_ConnectionRequest,			P2PSessionRequest_t,			m_callback_connection_request);
	STEAM_CALLBACK							(Steamworks_NetConnection, Callback_ConnectionRequestClient,	P2PSessionRequest_t,			m_callback_connection_request_client);
	STEAM_GAMESERVER_CALLBACK				(Steamworks_NetConnection, Callback_TicketAuthResult,			ValidateAuthTicketResponse_t,	m_callback_ticket_auth_result);	

protected:
	static std::vector<Steamworks_CooldownEntry> g_cooldown_list;

	static void AddSteamIdToCooldownList(CSteamID id);
	static bool IsSteamIdInCooldownList(CSteamID id);
	static void PollCooldownList();

	void CloseP2PSession();

protected:
	void Poll();
	void PollPendingPackets();
	void PollMainThread();
	void PollGlobals();

	Steamworks_NetConnection* New_Connection_Detected(CSteamID id);

	void SendPacket(const void* data, int data_length, EP2PSend send_type, int channel, CSteamID id);
	void RecievePacket(const void* buffer, int bufferLength, CSteamID source, int channel);
	bool IsSimulatingLatency();

public:
	Steamworks_NetConnection(Steamworks_NetConnection* parent = NULL);
	~Steamworks_NetConnection();
	
	NetConnection_Error::Type Validate_New_Connection(NetPacket_C2S_Connect* packet);

	OnlineUser* Get_Online_User();

	void Disconnect_Internal(NetConnection_Error::Type type = NetConnection_Error::NONE, bool bSupressDisconnectedPacket = false);

	bool Has_Same_Endpoint_As(NetConnection* other);

	bool Connect(OnlineUser* user, bool as_query_connection = false);
	bool Listen(NetAddress local_address);
	void Disconnect(NetConnection_Error::Type type = NetConnection_Error::NONE);
	void Send(NetPacket* packet, int channel = 0, bool reliable = false);
	void SendTo(NetPacket* packet, int channel, bool reliable, OnlineUser* user);
	void SendTo(NetPacket* packet, int channel, bool reliable, CSteamID id);

};

#endif

