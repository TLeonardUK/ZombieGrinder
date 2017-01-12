// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NETWORK_NET_CONNECTION_
#define _ENGINE_NETWORK_NET_CONNECTION_

#include "Engine/Network/NetConnectionTypes.h"
#include "Engine/Network/NetAddress.h"
#include "Engine/Network/NetPacket.h"
//#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/Packets/PacketDispatcher.h"

#include "Generic/Types/CircleBuffer.h"

#include "Engine/Online/OnlineUser.h"

class Mutex;
class NetPacket;
class NetFileTransfer;
class NetPacket_C2S_Pong;
class NetPacket_S2C_Ping;
class NetPacket_C2S_Connect;

struct PacketStat
{
	unsigned int type_id;
	int total_in;
	int total_out;
	int packets_in;
	int packets_out;
};

struct Partial_Packet
{
	DataBuffer payload;
};

class NetConnection
{
	MEMORY_ALLOCATOR(NetConnection, "Network");

protected:
	enum
	{
		connection_timeout			= 1000 * 20,
		ping_interval				= 200,
		ping_timeout				= 1000 * 10,
		ping_avg_samples			= (1000 * 2) / ping_interval	// 2 second ping average, every 10 ping packets.
	};

	HashTable<Partial_Packet, unsigned int>	m_partial_packets;

	bool m_dead;
	bool m_immediate_disconnect;
	double m_death_time;

	NetConnection_State::Type m_old_state;
	NetConnection_State::Type m_state_change_state;
	NetConnection_Error::Type m_error;
	NetConnection_State::Type m_state;
	double m_state_change_timer;

	std::vector<NetPacket*>	m_packets;
	Mutex* m_packet_mutex;

	std::vector<NetFileTransfer*> m_file_transfers;

	NetConnection* m_parent;
	std::vector<NetConnection*> m_children;

	NetPacket_C2S_Connect m_connect_packet;
	u64 m_source_lobby_id;

	bool m_has_users;

	bool m_listen_connection;

	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedPong, NetPacket_C2S_Pong, m_callback_recieve_pong);
	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedPing, NetPacket_S2C_Ping, m_callback_recieve_ping);
	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedFileTransferStart,  NetPacket_C2S_FileTransferStart, m_callback_file_transfer_start);
	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedFileTransferChunk,  NetPacket_C2S_FileTransferChunk, m_callback_file_transfer_chunk);
	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedFileTransferFinish, NetPacket_C2S_FileTransferFinish, m_callback_file_transfer_finish);
	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedFileTransferFinishAck, NetPacket_C2S_FileTransferFinishAck,	m_callback_file_transfer_finish_ack);

	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedConnected,	NetPacket_S2C_Connected,	m_callback_recieve_connected);
	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedDisconnect,	NetPacket_C2S_Disconnect,	m_callback_recieve_disconnect);

	PACKET_RECIEVE_CALLBACK(NetConnection, Callback_RecievedConnect,	NetPacket_C2S_Connect,		m_callback_recieve_connect);

	double m_last_pong_recieve_time;
	double m_last_ping_send_time;
	CircleBuffer<double, ping_avg_samples> m_ping_buffer;
	double m_ping;

	std::vector<NetPacket_EnforcedMod> m_missing_client_mods;
	std::vector<NetPacket_EnforcedMod> m_missing_server_mods;

	static double s_stat_timer;
	static std::vector<PacketStat> s_stats;

protected:
	friend class NetManager;

	void Set_State(NetConnection_State::Type state);
	void Set_Error(NetConnection_Error::Type error);

	bool Handle_Internal_Packet(NetPacket* packet);
	void Handle_Packet(NetPacket* packet);

	void Queue_Packet(NetPacket* packet);

	NetPacket* Parse_Net_Packet(const char* data, int size, bool force_full = false);

	void OnPacketIn(unsigned int type_id, int size);
	void OnPacketOut(unsigned int type_id, int size);

	static void UpdateStats();

	virtual void Poll();
	virtual void PollMainThread();
	virtual void PollGlobals();

	// Connections are owned by NetManager, don't try and delete
	// them yourself, call dispose! We have to do this to keep
	// thread-safety as connections are accessed by multiple threads.
	virtual ~NetConnection();

public:
	static NetConnection* Create();
	NetConnection(NetConnection* parent);

	void Register_File_Transfer(NetFileTransfer* transfer);
	void Unregister_File_Transfer(NetFileTransfer* transfer);
	NetFileTransfer* Get_File_Transfer(std::string slot, bool get_recieve);
	NetFileTransfer* Get_File_Transfer(int slot_hash, bool get_recieve);

	void Set_Client_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods);
	void Set_Server_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods);
	std::vector<NetPacket_EnforcedMod> Get_Server_Missing_Mods();
	std::vector<NetPacket_EnforcedMod> Get_Client_Missing_Mods();

	virtual bool Has_Same_Endpoint_As(NetConnection* other) = 0;

	void Dispose();
	bool Is_Dead();
	double Get_Dead_Duration();
	void Set_Immediate_Disconnect();
	bool Get_Immediate_Disconnect();

	float Get_Ping();
	float Get_Time_Since_Last_Contact();

	bool Has_State_Changed();
	NetConnection_State::Type Get_Old_State();
	NetConnection_State::Type Get_State();
	NetConnection_Error::Type Get_Error();
	std::string Get_Error_Message(NetConnection_Error::Type error);

	NetPacket* Recieve();

	void Broadcast(NetPacket* packet, int channel = 0, bool reliable = false, NetConnection* except = NULL);
	bool Is_Listen();
	bool Is_Child();	

	u64 Get_Source_Lobby_ID();
	NetPacket_C2S_Connect* Get_Connect_Packet();

	virtual NetConnection_Error::Type Validate_New_Connection(NetPacket_C2S_Connect* packet) = 0;
	virtual bool Connect(OnlineUser* user, bool as_query_connection = false) = 0;
	virtual bool Listen(NetAddress local_address) = 0;
	virtual void Disconnect(NetConnection_Error::Type type = NetConnection_Error::NONE) = 0;
	virtual OnlineUser* Get_Online_User() = 0;
	virtual void Send(NetPacket* packet, int channel = 0, bool reliable = false) = 0;
	virtual	void SendTo(NetPacket* packet, int channel, bool reliable, OnlineUser* user) = 0;

	static float GetInRate();
	static float GetOutRate();
	
};

#endif

