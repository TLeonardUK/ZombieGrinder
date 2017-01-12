// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetFileTransfer.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Platform/Platform.h"

#include "Engine/IO/BinaryStream.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Generic/Stats/Stats.h"
#include "Engine/Engine/EngineOptions.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Resources/PackageFile.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Network/BSD/BSD_NetConnection.h"
#include "Engine/Network/Steamworks/Steamworks_NetConnection.h"
#endif

#include "Generic/Helper/PersistentLogHelper.h"

DEFINE_TIME_AVERAGE_STATISTIC("Network/Packets In Per Second",		float,		1000.0f,			g_stat_network_packets_in_per_second,				true);
DEFINE_TIME_AVERAGE_STATISTIC("Network/Bytes In Per Second",		float,		1000.0f,			g_stat_network_bytes_in_per_second,					true);
DEFINE_TIME_AVERAGE_STATISTIC("Network/Packets Out Per Second",		float,		1000.0f,			g_stat_network_packets_out_per_second,				true);
DEFINE_TIME_AVERAGE_STATISTIC("Network/Bytes Out Per Second",		float,		1000.0f,			g_stat_network_bytes_out_per_second,				true);

//#define PRINT_PACKET_STATS

double NetConnection::s_stat_timer = 0.0f;
std::vector<PacketStat> NetConnection::s_stats;

NetConnection* NetConnection::Create()
{
#if defined(OPT_STEAM_PLATFORM)
#ifdef OPT_STEAM_BSD_SOCKETS
	return new BSD_NetConnection();
#else
	return new Steamworks_NetConnection();
#endif
#else
	#error "Online platform unsupported."
#endif
}

float NetConnection::GetInRate()
{
	return g_stat_network_bytes_in_per_second.To_Float();
}

float NetConnection::GetOutRate()
{
	return g_stat_network_bytes_out_per_second.To_Float();
}

NetConnection::NetConnection(NetConnection* parent)
	: m_parent(parent)
	, m_state(NetConnection_State::Idle)
	, m_error(NetConnection_Error::NONE)
	, m_state_change_timer(0.0f)
	, m_dead(false)
	, m_state_change_state(NetConnection_State::Idle)
	, m_old_state(NetConnection_State::Idle)
	, m_last_pong_recieve_time(0.0f)
	, m_listen_connection(false)
	, m_last_ping_send_time(0.0f)
	, m_ping(0.0f)
	, m_has_users(false)
	, m_source_lobby_id(0)
	, m_immediate_disconnect(false)
	, m_callback_recieve_pong(this, &NetConnection::Callback_RecievedPong, true, this)
	, m_callback_recieve_ping(this, &NetConnection::Callback_RecievedPing, true, this)
	, m_callback_file_transfer_start(this, &NetConnection::Callback_RecievedFileTransferStart,		true, this)
	, m_callback_file_transfer_chunk(this, &NetConnection::Callback_RecievedFileTransferChunk,		true, this)
	, m_callback_file_transfer_finish(this, &NetConnection::Callback_RecievedFileTransferFinish,		true, this)
	, m_callback_file_transfer_finish_ack(this, &NetConnection::Callback_RecievedFileTransferFinishAck,	true, this)
	, m_callback_recieve_disconnect(this, &NetConnection::Callback_RecievedDisconnect,	true, this)
	, m_callback_recieve_connected(this, &NetConnection::Callback_RecievedConnected,	true, this)
	, m_callback_recieve_connect(this, &NetConnection::Callback_RecievedConnect,		true, this)
{
	NetManager::Get()->Register_Connection(this);
	m_packet_mutex = Mutex::Create();
}

NetConnection::~NetConnection()
{
	if (m_parent != NULL)
	{
		std::vector<NetConnection*>::iterator iter = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
		if (iter != m_parent->m_children.end())
		{
			m_parent->m_children.erase(iter);
		}
	}

	while (m_children.size() > 0)
	{
		NetConnection* connection = *m_children.begin();
		connection->Disconnect(); 
		SAFE_DELETE(connection);
	}

	m_children.clear();
	m_partial_packets.Clear();

	SAFE_DELETE(m_packet_mutex);

	// Unregister all file transfers
	while (m_file_transfers.size() > 0)
	{
		Unregister_File_Transfer(*m_file_transfers.begin());
	}

	NetManager::Get()->Unregister_Connection(this);
}

void NetConnection::OnPacketIn(unsigned int type_id, int size)
{
	g_stat_network_bytes_in_per_second.Set((float)size);
	g_stat_network_packets_in_per_second.Set(1.0f);

#ifdef PRINT_PACKET_STATS
	for (std::vector<PacketStat>::iterator iter = s_stats.begin(); iter != s_stats.end(); iter++)
	{
		PacketStat& stat = *iter;
		if (stat.type_id == type_id)
		{
			stat.total_in += size;
			stat.packets_in += 1;
			return;
		}
	}

	PacketStat newstat;
	newstat.type_id = type_id;
	newstat.total_in = size;
	newstat.total_out = 0;
	newstat.packets_in = 1;
	newstat.packets_out = 0;
	s_stats.push_back(newstat);
#endif
}

void NetConnection::OnPacketOut(unsigned int type_id, int size)
{
	g_stat_network_bytes_out_per_second.Set((float)size);
	g_stat_network_packets_out_per_second.Set(1.0f);

#ifdef PRINT_PACKET_STATS
	for (std::vector<PacketStat>::iterator iter = s_stats.begin(); iter != s_stats.end(); iter++)
	{
		PacketStat& stat = *iter;
		if (stat.type_id == type_id)
		{
			stat.total_out += size;
			stat.packets_out += 1;
			return;
		}
	}

	PacketStat newstat;
	newstat.type_id = type_id;
	newstat.total_out = size;
	newstat.total_in = 0;
	newstat.packets_out = 1;
	newstat.packets_in = 0;
	s_stats.push_back(newstat);
#endif
}

void NetConnection::UpdateStats()
{
#ifdef PRINT_PACKET_STATS
	double ticks = Platform::Get()->Get_Ticks();
	if (ticks - s_stat_timer > 1000.0f)
	{
		for (std::vector<PacketStat>::iterator iter = s_stats.begin(); iter != s_stats.end(); iter++)
		{
			PacketStat& stat = *iter;
			NetPacket* packet = NetPacket::Create_By_Type_ID(stat.type_id);
			if (packet != NULL)
			{
				DBG_ONSCREEN_LOG(packet->Get_Type_ID(), Color::Green, 1.0f, "%s - In=%i Out=%i PIn=%i POut=%i", packet->Get_Name(), stat.total_in, stat.total_out, stat.packets_in, stat.packets_out);
				SAFE_DELETE(packet);
			}
		}
		
		s_stat_timer = ticks;
		s_stats.clear();
	}
#endif
}

void NetConnection::Poll()
{
}

void NetConnection::PollGlobals()
{
}

void NetConnection::PollMainThread()
{
	// Update by state.
	double ticks = Platform::Get()->Get_Ticks();
	switch (m_state)
	{
	case NetConnection_State::Connecting:
		{
			double elapsed = ticks - m_state_change_timer;
			if (elapsed > connection_timeout)
			{
				DBG_LOG("Cancelling connection, connection took to long (%.2f ms).", elapsed);

				Disconnect();
				Set_Error(NetConnection_Error::Timeout);

				return;
			}
			break;
		}

	case NetConnection_State::Connected:
		{
			// Not recieved a pong for a long time? Crap, probably disconnected.
#ifndef OPT_DISABLE_NET_TIMEOUTS
			double recieve_elapsed = ticks - m_last_pong_recieve_time;
			if (m_last_pong_recieve_time != 0)
			{
				if (recieve_elapsed > ping_timeout)
				{
					DBG_LOG("Closing connection ('%s'), ping timeout (%.2f ms).", Get_Connect_Packet() == NULL ? "unconnected" : Get_Connect_Packet()->Usernames[0].c_str(), recieve_elapsed);

					Disconnect();
					Set_Error(NetConnection_Error::Timeout);

					return;
				}
			}
#endif

			// Send the next ping.
			double send_elapsed = ticks - m_last_ping_send_time;
			if (m_last_ping_send_time == 0 || send_elapsed > ping_interval)
			{
				NetPacket_S2C_Ping packet;
				packet.Time = ticks;
				Send(&packet, NET_CONNECTION_CHANNEL_PING_UNRELIABLE, true);

				m_last_ping_send_time = ticks;
			}				

			break;
		}
	}

	for (std::vector<NetFileTransfer*>::iterator iter = m_file_transfers.begin(); iter != m_file_transfers.end(); iter++)
	{
		NetFileTransfer* t = *iter;
		t->Poll();
	}
}

void NetConnection::Register_File_Transfer(NetFileTransfer* transfer)
{
	for (std::vector<NetFileTransfer*>::iterator iter = m_file_transfers.begin(); iter != m_file_transfers.end(); iter++)
	{
		NetFileTransfer* t = *iter;
		if (t->Get_Slot_Hash() == transfer->Get_Slot_Hash() &&
			transfer->Is_Recieving() == t->Is_Recieving())
		{
			Unregister_File_Transfer(t);	
			break;
		}
	}

	DBG_LOG("Registering file transfer in slot '0x%08x' for connection 0x%08x.", transfer->Get_Slot_Hash(), this);
	m_file_transfers.push_back(transfer);
}

void NetConnection::Unregister_File_Transfer(NetFileTransfer* transfer)
{
	DBG_LOG("Unregistering file transfer in slot '0x%08x' for connection 0x%08x.", transfer->Get_Slot_Hash(), this);

	std::vector<NetFileTransfer*>::iterator iter = std::find(m_file_transfers.begin(), m_file_transfers.end(), transfer);
	if (iter != m_file_transfers.end())
	{
		m_file_transfers.erase(iter);
	}
}

NetFileTransfer* NetConnection::Get_File_Transfer(std::string slot, bool get_recieve)
{
	return Get_File_Transfer(StringHelper::Hash(slot.c_str()), get_recieve);
}

NetFileTransfer* NetConnection::Get_File_Transfer(int slot_hash, bool get_recieve)
{
	for (std::vector<NetFileTransfer*>::iterator iter = m_file_transfers.begin(); iter != m_file_transfers.end(); iter++)
	{
		NetFileTransfer* t = *iter;
		if (t->Get_Slot_Hash() == slot_hash)
		{
			if (t->Is_Recieving() == get_recieve)
			{
				return t;
			}
		}
	}

	return NULL;
}

void NetConnection::Dispose()
{
	DBG_ASSERT(m_dead == false);
	m_dead = true;
	m_death_time = Platform::Get()->Get_Ticks();
}

bool NetConnection::Is_Dead()
{
	return m_dead;
}

void NetConnection::Set_Immediate_Disconnect()
{
	m_immediate_disconnect = true;
}

bool NetConnection::Get_Immediate_Disconnect()
{
	return m_immediate_disconnect;
}

double NetConnection::Get_Dead_Duration()
{
	return Platform::Get()->Get_Ticks() - m_death_time;
}

float NetConnection::Get_Ping()
{
	return (float)m_ping;
}

float NetConnection::Get_Time_Since_Last_Contact()
{
	double ticks = Platform::Get()->Get_Ticks();
	double recieve_elapsed = ticks - m_last_pong_recieve_time;
	return (float)recieve_elapsed;
}

bool NetConnection::Has_State_Changed()
{
	bool changed = (m_state_change_state != Get_State());
	m_old_state = m_state_change_state;
	m_state_change_state = Get_State();
	return changed;
}

NetConnection_State::Type NetConnection::Get_Old_State()
{
	return m_old_state;
}

std::string NetConnection::Get_Error_Message(NetConnection_Error::Type error)
{
	switch (m_error)
	{
	case NetConnection_Error::NONE:					return "";
	case NetConnection_Error::Unknown:				return "#net_error_internal_error";
	case NetConnection_Error::Unavailable:			return "#net_error_unavailable";
	case NetConnection_Error::Timeout:				return "#net_error_timeout";

	case NetConnection_Error::Wrong_Version:		return "#net_error_wrong_version";
	case NetConnection_Error::Server_Full:			return "#net_error_server_full";
#ifdef OPT_PREMIUM_ACCOUNTS
	case NetConnection_Error::Server_Premium:		return "#net_error_server_premium";
#endif

	case NetConnection_Error::Banned:				return "#net_error_banned";
	case NetConnection_Error::Kicked:				return "#net_error_kicked";
	case NetConnection_Error::Lost_Connection:		return "#net_error_lost_connection";
	case NetConnection_Error::Duplicate_Connection:	return "#net_error_duplicate_connection";
	case NetConnection_Error::Transmission_Error:	return "#net_error_transmission_error";
	case NetConnection_Error::Server_Reserved:		return "#net_error_server_reserved";
	case NetConnection_Error::Invalid_Ticket:		return "#net_error_invalid_ticket";

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

	default:										return "#net_error_internal_error";
	}
}

void NetConnection::Set_State(NetConnection_State::Type state)
{
	m_state = state;
	m_state_change_timer = Platform::Get()->Get_Ticks();

	if (m_state == NetConnection_State::Connecting)
	{
		m_last_pong_recieve_time = Platform::Get()->Get_Ticks();
		m_partial_packets.Clear();
	}
}

void NetConnection::Set_Error(NetConnection_Error::Type error)
{
	m_error = error;
	Set_State(NetConnection_State::Error);
}

NetConnection_State::Type NetConnection::Get_State()
{
	return m_state;
}

NetConnection_Error::Type NetConnection::Get_Error()
{
	return m_error;
}

bool NetConnection::Is_Listen()
{
	return m_listen_connection;
}

bool NetConnection::Is_Child()
{
	return (m_parent != NULL);
}

bool NetConnection::Callback_RecievedPong(NetPacket_C2S_Pong* packet)
{
	double ticks = Platform::Get()->Get_Ticks();

	double elapsed = ticks - packet->Time;
	m_last_pong_recieve_time = ticks;

	m_ping_buffer.Push(elapsed);

	double average = 0.0f;
	for (int i = 0; i < m_ping_buffer.Size(); i++)
	{
		average += m_ping_buffer.Get(i);
	}

	m_ping = (average / m_ping_buffer.Size());

	//DBG_LOG("<< Pong %f (ping=%f avgs=%i) ms", elapsed, m_ping, m_ping_buffer.Size());

	return true;
}

bool NetConnection::Callback_RecievedPing(NetPacket_S2C_Ping* packet)
{
	NetPacket_C2S_Pong reply;
	reply.Time = packet->Time;
	Send(&reply, NET_CONNECTION_CHANNEL_PING_UNRELIABLE, true);

	return true;
}

NetPacket* NetConnection::Recieve()
{
	MutexLock lock(m_packet_mutex);

	if (m_packets.size() > 0)
	{
		NetPacket* packet = *m_packets.begin();
		m_packets.erase(m_packets.begin());
		return packet;
	}

	return NULL;
}

void NetConnection::Queue_Packet(NetPacket* packet)
{
	MutexLock lock(m_packet_mutex);
	m_packets.push_back(packet);
}

bool NetConnection::Handle_Internal_Packet(NetPacket* packet)
{
	return PacketDispatcher::Dispatch(packet, true);
}

void NetConnection::Handle_Packet(NetPacket* packet)
{
	// We shouldn't even be getting this wtf. If this happens
	// something has not been shut down correctly.
	if (m_state == NetConnection_State::Disconnected)
	{
		SAFE_DELETE(packet);
		return;
	}

	// We get to this state if a packet didn't parse correctly :(
	if (packet == NULL)
	{		
		Disconnect();
		Set_Error(NetConnection_Error::Transmission_Error);
		SAFE_DELETE(packet);
		return;
	}

	packet->Set_Server_Packet(!Is_Child());
	packet->Set_Recieved_From(this);

	// If its an internal packet we don't need to pass it on.
	if (Handle_Internal_Packet(packet))
	{
		SAFE_DELETE(packet);
		return;
	}

	// Store packet.
	Queue_Packet(packet);
}

void NetConnection::Broadcast(NetPacket* packet, int channel, bool reliable, NetConnection* except)
{
	for (std::vector<NetConnection*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		NetConnection* conn = *iter;

		if (conn != except)
		{
			conn->Send(packet, channel, reliable);
		}
	}
}

bool NetConnection::Callback_RecievedFileTransferStart(NetPacket_C2S_FileTransferStart* packet)
{
	DBG_LOG("Recieved file transfer start for slot '0x%08x', connection 0x%08x.", packet->slot_hash, this);

	NetFileTransfer* transfer = new NetFileTransfer(packet->slot_hash, this, true);
	transfer->Recieve_Start(packet);

	return true;
}

bool NetConnection::Callback_RecievedFileTransferChunk(NetPacket_C2S_FileTransferChunk* packet)
{
	//DBG_LOG("Recieved file transfer chunk for slot '0x%08x', connection 0x%08x.", packet->slot_hash, this);

	NetFileTransfer* transfer = Get_File_Transfer(packet->slot_hash, true);
	if (transfer != NULL)
	{
		transfer->Recieve_Chunk(packet);
	}

	return true;
}

bool NetConnection::Callback_RecievedFileTransferFinish(NetPacket_C2S_FileTransferFinish* packet)
{
	DBG_LOG("Recieved file transfer finish for slot '0x%08x', connection 0x%08x.", packet->slot_hash, this);

	NetFileTransfer* transfer = Get_File_Transfer(packet->slot_hash, true);
	if (transfer != NULL)
	{
		transfer->Recieve_Finish(packet);
	}

	return true;
}

bool NetConnection::Callback_RecievedFileTransferFinishAck(NetPacket_C2S_FileTransferFinishAck* packet)
{
	DBG_LOG("Recieved file transfer finish ack for slot '0x%08x', connection 0x%08x.", packet->slot_hash, this);

	NetFileTransfer* transfer = Get_File_Transfer(packet->slot_hash, false);
	if (transfer != NULL)
	{
		transfer->Recieve_Finish_Ack(packet);
	}

	return true;
}


void NetConnection::Set_Client_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods)
{
	m_missing_client_mods = mods;
}

void NetConnection::Set_Server_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods)
{
	m_missing_server_mods = mods;
}

std::vector<NetPacket_EnforcedMod> NetConnection::Get_Server_Missing_Mods()
{
	return m_missing_server_mods;
}

std::vector<NetPacket_EnforcedMod> NetConnection::Get_Client_Missing_Mods()
{
	return m_missing_client_mods;
}

bool NetConnection::Callback_RecievedConnected(NetPacket_S2C_Connected* packet)
{
	DBG_ASSERT(m_listen_connection == false);

	if (packet->Result == NetConnection_Error::NONE)
	{
		DBG_LOG("Recieved connected packet from server. We are now connected.");
		Set_State(NetConnection_State::Connected);
	}
	else
	{
		DBG_LOG("Recieved connected packet from server. Failed to connect, connection was rejected with error code '%i'.", packet->Result);
		Set_Client_Missing_Mods(packet->Missing_Client_Mods);
		Set_Server_Missing_Mods(packet->Missing_Server_Mods);
		Disconnect(packet->Result);
	}

	return true;
}

bool NetConnection::Callback_RecievedDisconnect(NetPacket_C2S_Disconnect* packet)
{
	if (m_listen_connection == true)
	{
		DBG_LOG("Recieved disconnected packet from client. Bye bye.");
		Disconnect();
	}
	else
	{
		DBG_LOG("Recieved disconnected packet from server with error code '%i'.", packet->Result);
		Disconnect(packet->Result);
		Set_Immediate_Disconnect();
	}

	return true;
}

NetPacket* NetConnection::Parse_Net_Packet(const char* data, int size, bool force_full)
{
	BinaryStream stream(data, size);
	unsigned int type_id = stream.Read<unsigned int>();
	unsigned char is_full = 0;
	int payload_size = 0;
	if (force_full == true)
	{
		 payload_size = (int)stream.Read<unsigned int>();
		 is_full = 1;
	}
	else
	{
		is_full = stream.Read<unsigned char>();
		payload_size = stream.Bytes_Remaining();
	}

	// Slerp up the packet!
	if (is_full != 0)
	{	
		NetPacket* packet = NetPacket::Create_By_Type_ID(type_id);
		if (packet == NULL)
		{
			DBG_LOG("Failed to handle packet, contains unknown packet type-id (0x%08x).", type_id);
			return NULL;
		}

		Partial_Packet* output;
		if (m_partial_packets.Get_Ptr(type_id, output))
		{
			int current_size = output->payload.Size();
			output->payload.Reserve(current_size + payload_size, true);
			stream.ReadBuffer(output->payload.Buffer(), current_size, payload_size);

			BinaryStream payload_stream(output->payload.Buffer(), output->payload.Size());

			m_partial_packets.Remove(type_id);

			if (!packet->Deserialize(&payload_stream))
			{
				DBG_LOG("Failed to deserialize packet (0x%08x), contains corrupted or invalid data.", type_id);
				SAFE_DELETE(packet);
				return NULL;
			}
		}
		else
		{	
			if (!packet->Deserialize(&stream))
			{
				DBG_LOG("Failed to deserialize packet (0x%08x), contains corrupted or invalid data.", type_id);
				SAFE_DELETE(packet);
				return NULL;
			}
		}

		return packet;
	}

	// Store fragment so we can put packet together.
	else
	{
		Partial_Packet* output;

		// Append to pre-existing packet.
		if (m_partial_packets.Get_Ptr(type_id, output))
		{
			int current_size = output->payload.Size();
			output->payload.Reserve(current_size + payload_size, true);
			stream.ReadBuffer(output->payload.Buffer(), current_size, payload_size);
		}

		// Store new partial packet.
		else
		{
			Partial_Packet packet;
			packet.payload.Reserve(payload_size);
			stream.ReadBuffer(packet.payload.Buffer(), 0, payload_size);

			m_partial_packets.Set(type_id, packet);
		}
	}

	return NULL;
}

bool NetConnection::Callback_RecievedConnect(NetPacket_C2S_Connect* packet)
{
	OnlineUser* user = Get_Online_User();

	DBG_LOG("Recieved connection packet from '%s'.", user->Get_Username().c_str());

	m_source_lobby_id = packet->Lobby_ID;

	NetConnection_Error::Type result = NetConnection_Error::NONE;

	// Check enforced guids.
	std::vector<PackageFile*> enforced_mods = GameEngine::Get()->Get_Runner()->Get_Server_Enforced_Packages();
	std::vector<NetPacket_EnforcedMod> mods_we_do_not_have;
	std::vector<NetPacket_EnforcedMod> mods_they_do_not_have;

	// Check which mods the user has that we do not.
	for (std::vector<NetPacket_EnforcedMod>::iterator iter = packet->Enforced_Mods.begin(); iter != packet->Enforced_Mods.end(); iter++)
	{
		bool bHave = false;
		NetPacket_EnforcedMod& mod = *iter;

		for (std::vector<PackageFile*> ::iterator iter2 = enforced_mods.begin(); iter2 != enforced_mods.end(); iter2++)
		{
			PackageFile* file = *iter2;
			std::string guid = file->Get_Header()->guid;

			if (guid == mod.GUID)
			{
				bHave = true;
				break;
			}
		}

		if (!bHave)
		{
			mods_we_do_not_have.push_back(mod);
		}
	}

	// Check which mods we have that the user does not.
	for (std::vector<PackageFile*> ::iterator iter2 = enforced_mods.begin(); iter2 != enforced_mods.end(); iter2++)
	{
		PackageFile* file = *iter2;
		std::string guid = file->Get_Header()->guid;

		bool bHave = false;

		for (std::vector<NetPacket_EnforcedMod>::iterator iter = packet->Enforced_Mods.begin(); iter != packet->Enforced_Mods.end(); iter++)
		{
			NetPacket_EnforcedMod& mod = *iter;

			if (guid == mod.GUID)
			{
				bHave = true;
				break;
			}
		}

		if (!bHave)
		{
			NetPacket_EnforcedMod mod;
			mod.GUID = file->Get_Header()->guid;
			mod.Workshop_ID = file->Get_Header()->workshop_id;
			mod.Workshop_Title = file->Get_Header()->workshop_name;
			mods_they_do_not_have.push_back(mod);
		}
	}

	// No longer hosting.
	if (NetManager::Get()->Server() == NULL)
	{
		DBG_LOG("Rejecting connection from '%s', no longer hosting server.", user->Get_Username().c_str());
		result = NetConnection_Error::Server_Reserved;
	}

	// Wrong game version?
	else if (packet->Version != 0 && packet->Version != NetManager::Get()->Get_Net_Version())
	{
		DBG_LOG("Rejecting connection from '%s', user has the wrong game version (has %i, expected %i).", user->Get_Username().c_str(), packet->Version, NetManager::Get()->Get_Net_Version());
		result = NetConnection_Error::Wrong_Version;
	}

	// Client mod missmatch.
	else if (mods_we_do_not_have.size() > 0)
	{
		DBG_LOG("Rejecting connection from '%s', user has enforced mods installed that we are not running.", user->Get_Username().c_str());
		result = NetConnection_Error::Server_Mod_Missmatch;
	}

	// Server mod missmatch.
	else if (mods_they_do_not_have.size() > 0)
	{
		DBG_LOG("Rejecting connection from '%s', user dosen't have all of our enforced mods installed.", user->Get_Username().c_str());
		result = NetConnection_Error::Client_Mod_Missmatch;
	}

	// Do we have enough space for all of the connections users?
	else if (NetManager::Get()->Server()->Available_Player_Slots() < packet->Local_User_Count)
	{
		DBG_LOG("Rejecting connection from '%s', user has %i local users but we only have %i available slots.", user->Get_Username().c_str(), packet->Local_User_Count, NetManager::Get()->Server()->Available_Player_Slots());
		result = NetConnection_Error::Server_Full;
	}

#ifdef OPT_PREMIUM_ACCOUNTS
	// Premium only?
	else if (*EngineOptions::server_premium && !packet->Premium_Account)
	{
		DBG_LOG("Rejecting connection from '%s', user does not have a premium account.", user->Get_Username().c_str());
		result = NetConnection_Error::Server_Premium;
	}
#endif

	// User already connected? Or duplicate connection packet?
#ifndef OPT_STEAM_ALLOW_DUPLICATE_CONNECTIONS
	else if (NetManager::Get()->Server()->Is_Connection_Duplicate(this) || (Get_State() == NetConnection_State::Connected && m_has_users == true)) // Allow upgrading from query connection.
	{
		DBG_LOG("Rejecting connection from '%s', user is already connected.", user->Get_Username().c_str());
		result = NetConnection_Error::Duplicate_Connection;
	}
#endif

	// Is the user banned?
	else if (NetManager::Get()->Server()->Is_Connection_Banned(this))
	{
		DBG_LOG("Rejecting connection from '%s', user is banned.", user->Get_Username().c_str());
		result = NetConnection_Error::Banned;
	}
	
	// Is the user joining from reserved lobby?
	else if (!NetManager::Get()->Server()->Is_Connection_Part_Of_Reservation(this))
	{
		DBG_LOG("Rejecting connection from '%s', user is not part of lobby reservation.", user->Get_Username().c_str());
		result = NetConnection_Error::Server_Reserved;
	}

	// Do sub-connection type check.
	if (result == NetConnection_Error::NONE)
	{
		result = Validate_New_Connection(packet);
	}

	// Success.
	if (result == NetConnection_Error::NONE)
	{
		DBG_LOG("Accepting connection from '%s', user has %i local users.", user->Get_Username().c_str(), packet->Local_User_Count);
	}

	// Send em the result!
	NetPacket_S2C_Connected res_packet;
	res_packet.Result = result;
	res_packet.Missing_Client_Mods = mods_they_do_not_have;
	res_packet.Missing_Server_Mods = mods_we_do_not_have;
	Send(&res_packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

	// And disconnect if it was an error.
	if (result != NetConnection_Error::NONE)
	{
		Disconnect(result);
	}
	else
	{		
		// This is kinda a hack - because we can have a connection persist on steamwworks if we create a query connection then 
		// immediately after create a normal connection, it will seem as though its the same thing. So we set this flag so we
		// always update the connection state when this occurs.
		m_state_change_state = NetConnection_State::Idle;

		Set_State(NetConnection_State::Connected);

		// Dis is the dumbs.
		m_connect_packet = *packet;
		m_has_users = (packet->Local_User_Count > 0);
	}

	// Update connection states.
	//NetManager::Get()->Poll_Connection_States();

	return true;
}

u64 NetConnection::Get_Source_Lobby_ID()
{
	return m_source_lobby_id;
}

NetPacket_C2S_Connect* NetConnection::Get_Connect_Packet()
{
	return &m_connect_packet;
}
