// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NET_SERVER_
#define _ENGINE_NET_SERVER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/Packets/PacketDispatcher.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class NetConnection;
class NetUser;
class NetPacket_C2S_Connect;
class OnlineMatching_LobbySettings;

struct NetServerBan
{
	std::string username;
	u64 id;
};

class NetServer
{
	MEMORY_ALLOCATOR(NetServer, "Network");

private:
	
protected:
	NetConnection*				m_listen_connection;

	std::vector<NetUser*>		m_disconnected_users;

	int							m_max_players;
	bool						m_private_game;
	bool						m_secure;

	bool						m_reset_settings;

	u64							m_reserved_lobby_id;
	double						m_reserved_lobby_time;

	int							m_net_id_counter;

	int							m_map_load_index;

	bool						m_joining_disabled;

	double						m_last_spawn_time;

	double						m_last_auto_change_time;

	std::vector<NetServerBan>	m_banned_ids;

	enum
	{
		reserved_lobby_timeout = 20 * 1000
	};
	
	PACKET_RECIEVE_CALLBACK(NetServer, Callback_RecievedReserveRequest, NetPacket_C2S_ReserveServer,  m_callback_recieve_reservation);
	PACKET_RECIEVE_CALLBACK(NetServer, Callback_RecievedNameChanged, NetPacket_C2S_NameChanged,  m_callback_recieve_name_changed);

public:
	NetServer();
	virtual ~NetServer();
	virtual bool Init();

	virtual bool Is_Secure();

	virtual void Set_Joining_Enabled(bool enabled);

	virtual void Save_Bans();
	virtual void Load_Bans();
	virtual void Unban_User(u64 id);

	virtual void Reset_Settings();

	INLINE std::vector<NetServerBan> Get_Bans()
	{
		return m_banned_ids;
	}

	// Connection information.
	virtual int Map_Load_Count();
	virtual int Total_Player_Slots();
	virtual int Available_Player_Slots();
	virtual bool Is_Connection_Banned(NetConnection* connection);
	virtual bool Is_Connection_Duplicate(NetConnection* connection);
	virtual bool Is_Connection_Part_Of_Reservation(NetConnection* connection);

	virtual void On_Connected(NetConnection* connection, NetPacket_C2S_Connect* packet);
	virtual void On_Disconnected(NetConnection* connection);

	virtual void SendPreInitialMapChangeInfo(NetUser* user) {}

	virtual void Reserve(u64 lobby_id, OnlineMatching_LobbySettings* settings);
	virtual void Force_Map_Change(const char* guid, u64 workshop_id, int seed, int dungeon_level_index);
	virtual void Restart_Map();
	virtual void Kick_User(NetUser* user);
	virtual void Ban_User(NetUser* user);

	virtual NetConnection* Get_Connection();

	u64 Get_Reserved_Lobby_ID();
	double Get_Reserved_Lobby_Time();

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time);

	// Events for derived classes to hook into.
	virtual void On_User_Joined(NetUser* user)	{ }
	virtual void On_User_Left(NetUser* user)	{ }
	virtual void On_User_Name_Change(NetUser* user, std::string old_username, std::string new_username) { }

};

#endif

