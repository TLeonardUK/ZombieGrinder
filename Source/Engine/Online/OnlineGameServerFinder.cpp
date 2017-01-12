// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineGameServerFinder.h"

#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Network/Packets/EnginePackets.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetClient.h"

#include <algorithm>

// Generic sort predicate, sorts by lowest to highest ping.
bool Sort_Servers_By_Ping(OnlineMatching_Server a, OnlineMatching_Server b)
{
	return (a.Ping < b.Ping);
}

OnlineGameServerFinder::OnlineGameServerFinder()
	: m_state(OnlineGameServerFinder_State::Idle)
	, m_callback_recieve_result(this, &OnlineGameServerFinder::Callback_RecievedResult,	false, NULL)
	, m_connection(NULL)
{
}

OnlineGameServerFinder::~OnlineGameServerFinder()
{
	if (m_connection != NULL)
	{
		m_connection->Set_Immediate_Disconnect();
		m_connection->Disconnect();
		m_connection = NULL;
	}
}

void OnlineGameServerFinder::Begin_Search(int max_ping, int search_timeout, OnlineMatching_LobbySettings settings)
{
	if (m_state != OnlineGameServerFinder_State::Idle)
	{
		DBG_LOG("[Online Game Server Finder] Attempt to begin search while old search in progress, cancelling old search.");
		Cancel_Search();
	}

	DBG_LOG("[Online Game Server Finder] Searching for %s game servers with lower than %ims (0=any) ping", settings.Secure ? "secure" : "non-secure", max_ping);

	m_max_ping = max_ping;
	m_search_timeout = search_timeout;
	m_search_timer = 0;
	m_settings = settings;

	if (settings.Secure == true)
	{
		m_source = OnlineMatching_ServerSource::Reservable_Secure;
	}
	else
	{
		m_source = OnlineMatching_ServerSource::Reservable_None_Secure;
	}

	OnlineMatching::Get()->Refresh_Server_List(m_source);
	m_state = OnlineGameServerFinder_State::Searching;
}

void OnlineGameServerFinder::Cancel_Search()
{
	DBG_LOG("[Online Game Server Finder] Cancelled search.");

	OnlineMatching::Get()->Cancel_Server_List_Refresh(m_source);
	m_state = OnlineGameServerFinder_State::Idle;
}

OnlineGameServerFinder_State::Type OnlineGameServerFinder::Get_State()
{
	return m_state;
}

OnlineMatching_Server OnlineGameServerFinder::Get_Reserved_Server()
{
	return m_reserved_server;
}

bool OnlineGameServerFinder::Callback_RecievedResult(NetPacket_S2C_ReservationResult* packet)
{
	if (m_connection == NULL)
	{
		return false;
	}

	if (packet->Result == true)
	{
		DBG_LOG("[Online Game Server Finder] Server '%s' has been reserved for us!", m_reserve_server.Name.c_str());

		m_reserved_server = m_reserve_server; 
		m_state = OnlineGameServerFinder_State::Found_Server;
	}
	else
	{
		DBG_LOG("[Online Game Server Finder] Server '%s' could not be reserved for us :(.", m_reserve_server.Name.c_str());
	}

	m_connection->Set_Immediate_Disconnect();
	m_connection->Disconnect();
	m_connection = NULL;
	m_sent_reserve_request = false;

	return true;
}

void OnlineGameServerFinder::Tick(const FrameTime& time)
{
	switch (m_state)
	{
	case OnlineGameServerFinder_State::Idle:
		{
			break;
		}
	case OnlineGameServerFinder_State::Searching:
		{
			m_search_timer += time.Get_Frame_Time();

			if (!OnlineMatching::Get()->Is_Server_List_Refreshing(m_source) || 
				m_search_timer >= m_search_timeout)
			{
				std::vector<OnlineMatching_Server> servers = OnlineMatching::Get()->Get_Server_List(m_source);
				
				DBG_LOG("[Online Game Server Finder] Finished retrieving server list (%i total).", servers.size());

				m_potential_servers.clear();

				// Grab all servers with no players and a low ping as potential servers to join.
				for (std::vector<OnlineMatching_Server>::iterator iter = servers.begin(); iter != servers.end(); iter++)
				{
					OnlineMatching_Server server = *iter;
					if (server.PlayerCount == 0 && server.Ping < m_max_ping)
					{
						m_potential_servers.push_back(server);
					}
				}
				
				// Sort list from low ping to high ping.
				std::sort(m_potential_servers.begin(), m_potential_servers.end(), Sort_Servers_By_Ping);

				// Remove servers if we have to many to try.
				while (m_potential_servers.size() > max_reserve_attempts)
				{
					m_potential_servers.pop_back();
				}

				// Now start trying to reserve them!
				if (m_potential_servers.size() > 0)
				{					
					DBG_LOG("[Online Game Server Finder] %i servers are potential matches.", m_potential_servers.size());
					m_state = OnlineGameServerFinder_State::Reserving;
				}
				else
				{
					DBG_LOG("[Online Game Server Finder] No potential server matches were found.", m_potential_servers.size());
					m_state = OnlineGameServerFinder_State::No_Server_Found;
				}
			}
			break;
		}
	case OnlineGameServerFinder_State::Reserving:
		{
			if (m_connection == NULL)
			{
				if (m_potential_servers.size() == 0)
				{
					DBG_LOG("[Online Game Server Finder] No potential server matches were found.", m_potential_servers.size());
					m_state = OnlineGameServerFinder_State::No_Server_Found;
				}
				else
				{
					// Get net server we need to try and reserve.
					m_reserve_server = m_potential_servers.at(m_potential_servers.size() - 1);
					m_potential_servers.pop_back();

					// Dispose of connection if we have one from previous attempt.
					if (m_connection != NULL)
					{
						m_connection->Set_Immediate_Disconnect();
						m_connection->Disconnect();
						m_connection = NULL;
					}

					// Attempt to connect!
					m_connection = NetManager::Get()->Client()->Get_Connection();
					if (!m_connection->Connect(m_reserve_server.User, true))
					{
						DBG_LOG("[Online Game Server Finder] Failed to create connection to potentital matching server '%s'.", m_reserve_server.Name.c_str());

						m_connection->Set_Immediate_Disconnect();
						m_connection->Disconnect();
						m_connection = NULL;
					}

					m_sent_reserve_request = false;
				}
			}
			else
			{
				switch (m_connection->Get_State())
				{
				case NetConnection_State::Connected:
					{
						// Send request to reserve.
						if (m_sent_reserve_request == false)
						{
							DBG_LOG("[Online Game Server Finder] Connected to potential matching server '%s', sending reserve packet.", m_reserve_server.Name.c_str());
	
							NetPacket_C2S_ReserveServer packet;
							packet.LobbyID		= OnlineMatching::Get()->Get_Lobby_ID();
							packet.MapGUID		= m_settings.MapGUID;
							packet.MapWorkshopID= m_settings.MapWorkshopID;
							packet.MaxPlayers	= m_settings.MaxPlayers;
							packet.Private		= m_settings.Private;
							m_connection->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);

							m_sent_reserve_request = true;
							m_reserve_timeout = Platform::Get()->Get_Ticks();
						}
						else
						{
							if (Platform::Get()->Get_Ticks() - m_reserve_timeout > reserve_timeout)
							{
								DBG_LOG("[Online Game Server Finder] Timed out while trying to reserve server '%s'.", m_reserve_server.Name.c_str());

								m_connection->Set_Immediate_Disconnect();
								m_connection->Disconnect();
								m_connection = NULL;
							}
						}

						break;
					}
				case NetConnection_State::Connecting:
					{
						// Dum de dum do, *twiddles thumbs*.
						break;
					}
				default:
					{
						DBG_LOG("[Online Game Server Finder] Failed to connect to potential matching server '%s'.", m_reserve_server.Name.c_str());

						m_connection->Set_Immediate_Disconnect();
						m_connection->Disconnect();
						m_connection = NULL;
						
						break;
					}
				}
			}

			break;
		}
	case OnlineGameServerFinder_State::No_Server_Found:
		{
			break;
		}
	}
}
