// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NET_MANAGER_
#define _ENGINE_NET_MANAGER_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include "Generic/Threads/Thread.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include <string>
#include <vector>

class NetClient;
class NetServer;
class NetConnection;
class NetUser;
class OnlineUser;

#define NET_CLIENT_CODE(x) \
{ \
	NetClient* client = NetManager::Get()->Get_Client(); \
	if (client != NULL) \
	{ \
		x \
	} \
} \
	
#define NET_SERVER_CODE(x) \
{ \
	NetServer* server = NetManager::Get()->Get_Server(); \
	if (server != NULL) \
	{ \
		x \
	} \
} \
	
#define NET_CLIENT_AND_SERVER_CODE(x) \
{ \
	NetServer* server = NetManager::Get()->Get_Server(); \
	NetClient* client = NetManager::Get()->Get_Client(); \
	if (client != NULL || server != NULL) \
	{ \
		x \
	} \
} \

class NetManager : public Singleton<NetManager>
{
	MEMORY_ALLOCATOR(NetManager, "Network");

protected:
	std::vector<NetConnection*> m_connections;

	Mutex* m_connection_mutex;
	Thread* m_poll_thread;

	bool m_polling_paused;

	bool m_map_load_pending;
	int m_map_seed;
	int m_map_load_index;
	int m_map_dungeon_level;
	std::string m_map_guid;
	u64 m_map_workshop_id;

	bool m_running;

	std::vector<NetUser*> m_net_users;

protected:
	friend class NetConnection;

	// Connection polling thread entry point.
	static void Static_Thread_Entry_Point(Thread* self, void* ptr)
	{
		reinterpret_cast<NetManager*>(ptr)->Thread_Entry_Point(self, ptr);
	}
	void Thread_Entry_Point(Thread* self, void* ptr);

	// Connection management.
	void Register_Connection(NetConnection* connection);
	void Unregister_Connection(NetConnection* connection);

	enum
	{
		dead_connection_dispose_delay = 1000 // We delay disposal of dead connections to allow disconnect messages to get through.
	};

public:	
	NetManager();
	virtual ~NetManager();

	// Initialize base code.
	virtual bool Init();
	virtual void DeInit();

	// Initializes client-side network code.
	virtual bool Init_Client() = 0;	
	virtual bool Destroy_Client() = 0;

	// Initializes server-side network code.
	virtual bool Init_Server() = 0;
	virtual bool Destroy_Server() = 0;

	// Retrieves client.
	virtual NetClient* Client() = 0;

	// Retrieves client.
	virtual NetServer* Server() = 0;

	INLINE bool Is_Server()
	{
		return (Server() != NULL);
	}
	INLINE bool Is_Client()
	{
		return (Client() != NULL);
	}
	INLINE bool Is_Dedicated_Server()
	{
		return Is_Server() && !Is_Client();
	}

	// Gets the current version of net-code, used to make sure people on old
	// versions of the game don't try to connect to us!
	virtual unsigned int Get_Net_Version() = 0;

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time);

	// Gets how many active network connections there are.
	virtual int Get_Connection_Count();
	virtual std::vector<NetConnection*>& Get_Connections();
	virtual void Pause_Connection_Polling();
	virtual void Resume_Connection_Polling();
	virtual void Poll_Connections(bool no_state_changes = false);
	virtual void Poll_Connection_States();

	// Get/Set current map.
	virtual void Change_Map_By_GUID(const char* guid, u64 workshop_id, int seed, int load_index, int dungeon_level);
	virtual std::string Get_Current_Map_GUID();
	virtual std::string Get_Current_Short_Map_Name();
	virtual u64 Get_Current_Map_Workshop_ID();
	virtual int Get_Current_Map_Seed();
	virtual int Get_Current_Map_Load_Index();
	virtual int Get_Current_Map_Dungeon_Level();
	virtual bool Is_Map_Load_Pending();
	virtual void Clear_Map_Load_Pending();

	virtual void Set_Map_GUID(std::string guid, u64 workshop_id);

	virtual void Leave_Network_Game();

	// User creation.
	virtual NetUser* Create_Net_User(NetConnection* connection, int local_user_index, OnlineUser* user = NULL);
	virtual std::vector<NetUser*>& Get_Net_Users();
	virtual void Delete_Net_User(NetUser* user);
	virtual NetUser* Get_User_By_Net_ID(int id);
	virtual NetUser* Get_User_By_Username(std::string name);
	virtual NetUser* Get_User_By_Connection(NetConnection* connection);

	virtual int Get_Primary_Net_User_Count();

	virtual std::vector<NetUser*> Get_Local_Net_Users();
	virtual NetUser* Get_Primary_Local_Net_User();

};

#endif

