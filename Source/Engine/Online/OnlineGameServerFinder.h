// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_GAME_SERVER_FINDER_
#define _ENGINE_ONLINE_GAME_SERVER_FINDER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Network/NetAddress.h"

#include "Engine/Online/OnlineMatching.h"

#include "Generic/Events/Event.h"

#include "Engine/Engine/FrameTime.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/Packets/PacketDispatcher.h"

#include <string>
#include <vector>

class NetConnection;

struct OnlineGameServerFinder_State
{
	enum Type
	{
		Idle,
		Searching,
		Reserving,
		No_Server_Found,
		Found_Server,
		
		COUNT
	};
};

class OnlineGameServerFinder
{
	MEMORY_ALLOCATOR(OnlineGameServerFinder, "Network");

private:
	OnlineGameServerFinder_State::Type m_state;

	std::vector<OnlineMatching_Server> m_potential_servers;

	OnlineMatching_Server m_reserved_server;

	int m_max_ping;
	int m_search_timeout;
	bool m_secure;

	OnlineMatching_ServerSource::Type m_source;

	OnlineMatching_Server m_reserve_server;
	bool m_sent_reserve_request;

	double m_search_timer;

	double m_reserve_timeout;

	NetConnection* m_connection;

	OnlineMatching_LobbySettings m_settings;
	
	PACKET_RECIEVE_CALLBACK(OnlineGameServerFinder, Callback_RecievedResult, NetPacket_S2C_ReservationResult, m_callback_recieve_result);

	enum
	{
		reserve_timeout = 1000 * 10,
		max_reserve_attempts = 6
	};

public:

public:

	OnlineGameServerFinder();
	~OnlineGameServerFinder();
	
	// Begins searching for a server.
	void Begin_Search(int max_ping, int search_timeout, OnlineMatching_LobbySettings settings);

	// Cancels searching for a server.
	void Cancel_Search();

	// Gets current state of search.
	OnlineGameServerFinder_State::Type Get_State();

	// Gets the server to connect to.
	OnlineMatching_Server Get_Reserved_Server();

	// Ticks the state.
	void Tick(const FrameTime& time);

};

#endif

