// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NET_CLIENT_
#define _ENGINE_NET_CLIENT_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/Packets/PacketDispatcher.h"

#include "Engine/Online/OnlineMatching.h"

#include <string>
#include <vector>

class NetConnection;
class NetUser;

struct NetClient_State
{
	enum Type
	{
		Idle,

		Connecting,
		Connected,

		Error,
	};
};

struct NetClient_Error
{
	enum Type
	{
		NONE,
		Unknown,
		Unavailable,
		Timeout,

		Wrong_Version,		
		Server_Full,		
#ifdef OPT_PREMIUM_ACCOUNTS
		Server_Premium,
#endif
		Banned,
		Kicked,
		Lost_Connection,
		Duplicate_Connection,
		Transmission_Error,
		Server_Reserved,
		Invalid_Ticket,
		Client_Mod_Missmatch,
		Server_Mod_Missmatch
	};
};

class NetClient
{
	MEMORY_ALLOCATOR(NetClient, "Network");

private:
	NetClient_State::Type m_state;
	NetClient_Error::Type m_error;

	OnlineMatching_Server m_server;

	NetConnection* m_connection;

	std::vector<NetPacket_EnforcedMod> m_missing_client_mods;
	std::vector<NetPacket_EnforcedMod> m_missing_server_mods;

	PACKET_RECIEVE_CALLBACK		  (NetClient, Callback_RecievedUserListUpdate,	NetPacket_S2C_UserListUpdate,	m_callback_recieve_user_list_update);
	PACKET_RECIEVE_CLIENT_CALLBACK(NetClient, Callback_RecievedNameChanged,		NetPacket_C2S_NameChanged,		m_callback_recieve_name_changed);
	PACKET_RECIEVE_CLIENT_CALLBACK(NetClient, Callback_RecievedInitialState,	NetPacket_S2C_InitialState,		m_callback_recieve_initial_State);
	PACKET_RECIEVE_CLIENT_CALLBACK(NetClient, Callback_RecievedChangeMap,		NetPacket_S2C_ChangeMap,		m_callback_recieve_change_map);
	PACKET_RECIEVE_CLIENT_CALLBACK(NetClient, Callback_RecievedKick,			NetPacket_S2C_Kick,				m_callback_recieve_kick);	

protected:
	void Set_State(NetClient_State::Type state);
	void Set_Error(NetClient_Error::Type error);

	void Tick_Connecting(const FrameTime& time);
	void Tick_Connected(const FrameTime& time);

public:
	NetClient();
	~NetClient();

	virtual bool Init();
	
	virtual NetConnection* Get_Connection();

	void Set_Client_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods);
	void Set_Server_Missing_Mods(std::vector<NetPacket_EnforcedMod> mods);
	std::vector<NetPacket_EnforcedMod> Get_Server_Missing_Mods();
	std::vector<NetPacket_EnforcedMod> Get_Client_Missing_Mods();

	// Connection information.
	virtual void On_Connected(NetConnection* connection);
	virtual void On_Disconnected(NetConnection* connection);
	virtual void On_Name_Changed(OnlineUser* user, std::string old_username, std::string new_username);

	// Error information.
	virtual NetClient_State::Type Get_State();
	virtual NetClient_Error::Type Get_Error();
	virtual std::string Get_Error_Message(NetClient_Error::Type error);

	// Connect to servaaaa.
	virtual void Connect_To_Server(OnlineMatching_Server server);
	virtual void Disconnect_From_Server();

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time);

	// Events for derived classes to hook into.
	virtual void On_User_Joined(NetUser* user)	{ }
	virtual void On_User_Left(NetUser* user)	{ }

};

#endif

