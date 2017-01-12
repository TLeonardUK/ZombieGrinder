// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Network/Packets/PacketDispatcher.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Engine/Network/Packets/EnginePackets.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/MapFileHandle.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#include "Engine/Engine/GameEngine.h"

#include <algorithm>

NetManager::NetManager()
	: m_running(false)
	, m_poll_thread(NULL)
	, m_polling_paused(false)
	, m_map_load_pending(false)
	, m_map_seed(0)
	, m_map_load_index(0)
	, m_map_dungeon_level(0)
{
	Register_Engine_Packets();

	m_connection_mutex = Mutex::Create();

	PacketDispatcher::Init();
}

NetManager::~NetManager()
{
}

void NetManager::DeInit()
{
	DBG_LOG("Shutting down network manager ...");
	m_running = false;

	if (m_poll_thread != NULL)
		m_poll_thread->Join();
		
	while (m_connections.size() > 0)
	{
		NetConnection* connection = m_connections.back();
		m_connections.pop_back();
		SAFE_DELETE(connection);
	}

	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* connection = *iter;
		connection->m_connection = NULL;
		SAFE_DELETE(connection);
	}

	SAFE_DELETE(m_connection_mutex);
}

void NetManager::Register_Connection(NetConnection* connection)
{
	MutexLock lock(m_connection_mutex);

	m_connections.push_back(connection);
}

void NetManager::Unregister_Connection(NetConnection* connection)
{
	MutexLock lock(m_connection_mutex);

	std::vector<NetConnection*>::iterator iter = std::find(m_connections.begin(), m_connections.end(), connection);
	if (iter != m_connections.end())
	{
		m_connections.erase(iter);
	}
}

int NetManager::Get_Connection_Count()
{
	MutexLock lock(m_connection_mutex);
	return m_connections.size();
}

void NetManager::Pause_Connection_Polling()
{
	MutexLock lock(m_connection_mutex);
	m_polling_paused = true;
}

void NetManager::Resume_Connection_Polling()
{
	MutexLock lock(m_connection_mutex);
	m_polling_paused = false;
}

void NetManager::Thread_Entry_Point(Thread* self, void* ptr)
{
	while (m_running == true)
	{	
		bool no_connections = false;

		{
			MutexLock lock(m_connection_mutex);

			if (m_polling_paused == false)
			{
				no_connections = (m_connections.size() == 0);

				// Poll connections.
				for (unsigned int i = 0; i < m_connections.size(); i++) // Don't use iterator, may become invalid when polling connections.
				{
					NetConnection* connection = m_connections.at(i);
					connection->Poll();
				}
			}
		}

		NetConnection::UpdateStats();

		if (no_connections == true)
		{
			self->Sleep(50.0f); // Could use a semaphore, but I'm lazy and this saves cpu time fine.
		}
		else
		{
			self->Sleep((float)*EngineOptions::net_poll_interval);
		}
	}
}

bool NetManager::Init()
{
	m_poll_thread = Thread::Create("NetManager", &NetManager::Static_Thread_Entry_Point, this);
	if (m_poll_thread == NULL)
	{
		return false;
	}

	m_running = true;
	m_poll_thread->Start();

	return true;
}

void NetManager::Leave_Network_Game()
{
	Client()->Disconnect_From_Server();
	OnlineMatching::Get()->Leave_Lobby();
	GameEngine::Get()->Stop_Local_Server();

	// Delete all net-users.
	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* user = *iter;
		DBG_LOG("Cleaning up net user %s", user->Get_Username().c_str());
		SAFE_DELETE(user);
	}

	m_net_users.clear();
}

void NetManager::Poll_Connections(bool no_state_changes)
{
	{		
		MutexLock lock(m_connection_mutex);
		
		bool bPolledGlobals = false;

		for (std::vector<NetConnection*>::iterator iter = m_connections.begin(); iter != m_connections.end(); )
		{
			NetConnection* connection = *iter;

			if (!bPolledGlobals)
			{
				connection->PollGlobals();
				bPolledGlobals = true;
			}
			connection->PollMainThread();
			
			// Dispose of dead connections. We do this on this thread
			// to make sure the game thread isn't trying to access anything
			// when we delete them :3
			if (connection->Is_Dead())
			{
				if (connection->Get_Dead_Duration() > dead_connection_dispose_delay || connection->Get_Immediate_Disconnect())
				{
					DBG_LOG("Disposing of dead connection (dead for %f, immediate=%i).", connection->Get_Dead_Duration(), connection->Get_Immediate_Disconnect());
					SAFE_DELETE(connection);

					// Restart seeing as the delete will have busted up m_connections :(
					iter = m_connections.begin();
					continue;
				}
				else
				{
					iter++;
				}
			}
			else
			{
				// Dispatch all game-packets from this connection.
				NetPacket* packet = connection->Recieve();
				while (packet != NULL)
				{
					PacketDispatcher::Dispatch(packet, false);
					SAFE_DELETE(packet);

					packet = connection->Recieve();
				}

				iter++;
			}
		}	

		if (!no_state_changes)
			Poll_Connection_States();
	}
}

void NetManager::Poll_Connection_States()
{
	for (std::vector<NetConnection*>::iterator iter = m_connections.begin(); iter != m_connections.end(); iter++)
	{
		NetConnection* connection = *iter;

		// If connection state has changed, notify client/server of state-change.
		// Again, this is for multi-threading, we don't want to call it when the state
		// actually changes as it will come from the net-thread.
		if (connection->Has_State_Changed())	
		{
			bool has_connected = false;
			bool has_disconnected = false;

			DBG_LOG("Connection 0x%08x state changed.", connection);

			// Are we now connected?
			if (connection->Get_State() == NetConnection_State::Connected)
			{
				if (connection->Is_Listen() || connection->Is_Child())
				{
					Server()->On_Connected(connection, connection->Get_Connect_Packet());
				}
				else
				{
					Client()->On_Connected(connection);
				}
			}

			// Did we used to be connected?
			else if (connection->Get_Old_State() == NetConnection_State::Connected)
			{
				if (connection->Is_Listen() || connection->Is_Child())
				{
					Server()->On_Disconnected(connection);
				}
				else
				{
					Client()->On_Disconnected(connection);
				}
			}
		}
	}
}

void NetManager::Tick(const FrameTime& time)
{
	{
		PROFILE_SCOPE("Client Tick");

		if (Client() != NULL)
			Client()->Tick(time);
	}

	{
		PROFILE_SCOPE("Server Tick");

		if (Server() != NULL)
			Server()->Tick(time);
	}
	
	
	{
		PROFILE_SCOPE("User Tick");

		// Tick all of the net-users.
		for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
		{
			NetUser* user = *iter;
			user->Tick(time);
		}
	}
	
	{
		PROFILE_SCOPE("Poll Connections");

		Poll_Connections();
	}
}

void NetManager::Change_Map_By_GUID(const char* guid, u64 workshop_id, int seed, int load_index, int dungeon_level)
{
	m_map_guid = guid;
	m_map_workshop_id = workshop_id;
	m_map_seed = seed;
	m_map_load_pending = true;
	m_map_load_index = load_index;
	m_map_dungeon_level = dungeon_level;

	DBG_LOG("Network manager has changed current map to '%s' (workshopid=%llu, seed=0x%08x, index=%i, dungeon_level=%i) ...", guid, workshop_id, seed, load_index, m_map_dungeon_level);
}

void NetManager::Set_Map_GUID(std::string guid, u64 workshop_id)
{
	DBG_LOG("Map name changed to '%s' (workshopid = %llu).", guid.c_str(), workshop_id);

	m_map_guid = guid;
	m_map_workshop_id = workshop_id;
}

std::string  NetManager::Get_Current_Map_GUID()
{
	return m_map_guid;
}

int NetManager::Get_Current_Map_Dungeon_Level()
{
	return m_map_dungeon_level;
}

std::string  NetManager::Get_Current_Short_Map_Name()
{
	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(m_map_guid.c_str());
	if (handle != NULL)
	{
		return handle->Get()->Get_Header()->Short_Name;
	}
	else
	{
		return "";
	}
}

u64 NetManager::Get_Current_Map_Workshop_ID()
{
	return m_map_workshop_id;
}

int NetManager::Get_Current_Map_Seed()
{
	return m_map_seed;
}

int NetManager::Get_Current_Map_Load_Index()
{
	return m_map_load_index;
}

bool NetManager::Is_Map_Load_Pending()
{
	return m_map_load_pending;
}

void NetManager::Clear_Map_Load_Pending()
{
	m_map_load_pending = false;
}

NetUser* NetManager::Create_Net_User(NetConnection* connection, int local_user_index, OnlineUser* user)
{
	if (user == NULL)
	{
		user = connection->Get_Online_User();
	}

	NetUser* netuser = new NetUser(connection, local_user_index, user);
	m_net_users.push_back(netuser);

	DBG_LOG("Creating new network user.");

	return netuser;
}

void NetManager::Delete_Net_User(NetUser* user)
{
	std::vector<NetUser*>::iterator iter = std::find(m_net_users.begin(), m_net_users.end(), user);
	DBG_ASSERT(iter != m_net_users.end());
	
	m_net_users.erase(iter);

	DBG_LOG("Deleted network user.");

	SAFE_DELETE(user);
}

NetUser* NetManager::Get_User_By_Net_ID(int id)
{	
	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* connection = *iter;
		if (id == connection->Get_Net_ID())
		{
			return connection;
		}
	}
	return NULL;
}
NetUser* NetManager::Get_User_By_Connection(NetConnection* connection)
{	
	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* user = *iter;
		if (connection == user->Get_Connection() && user->Get_Local_User_Index() == 0)
		{
			return user;
		}
	}
	return NULL;
}

NetUser* NetManager::Get_User_By_Username(std::string name)
{
	name = StringHelper::Lowercase(name.c_str());

	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* connection = *iter;
		if (name == StringHelper::Lowercase(connection->Get_Username().c_str()))
		{
			return connection;
		}
	}

	return NULL;
}

std::vector<NetUser*>& NetManager::Get_Net_Users()
{
	return m_net_users;
}

std::vector<NetConnection*>& NetManager::Get_Connections()
{
	return m_connections;
}

std::vector<NetUser*> NetManager::Get_Local_Net_Users()
{
	std::vector<NetUser*> result;
	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* connection = *iter;
		if (connection->Get_Online_User()->Is_Local())
		{
			result.push_back(connection);
		}
	}
	return result;
}

NetUser* NetManager::Get_Primary_Local_Net_User()
{
	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* connection = *iter;
		if (connection->Get_Local_User_Index() == 0 && connection->Get_Online_User()->Is_Local())
		{
			return connection;
		}
	}
	return NULL;
}

int NetManager::Get_Primary_Net_User_Count()
{
	int count = 0;
	for (std::vector<NetUser*>::iterator iter = m_net_users.begin(); iter != m_net_users.end(); iter++)
	{
		NetUser* connection = *iter;
		if (connection->Get_Local_User_Index() == 0)
		{
			count++;
		}
	}
	return count;
}