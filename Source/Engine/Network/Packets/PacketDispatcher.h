// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NETWORK_PACKET_DISPATCHER_
#define _ENGINE_NETWORK_PACKET_DISPATCHER_

#include "Engine/Network/NetPacket.h"
#include "Engine/Network/Packets/EnginePackets.h"


#include "Generic/Types/DataBuffer.h"

#include "public/steam/steam_api.h"
#include "public/steam/steam_gameserver.h"

#include <vector>

class Mutex;
class NetConnection;
class PacketDispatcherCallbackBase;

struct PacketCallbackSource
{
	enum Type
	{
		Client		= 1,
		Server		= 2,
		Anything	= Client | Server
	};
};

#define PACKET_RECIEVE_CALLBACK(thisclass, func, param, var) PacketDispatcherCallback<thisclass, param, PacketCallbackSource::Anything> var; bool func(param* pParam)
#define PACKET_RECIEVE_CLIENT_CALLBACK(thisclass, func, param, var) PacketDispatcherCallback<thisclass, param, PacketCallbackSource::Client> var; bool func(param* pParam)
#define PACKET_RECIEVE_SERVER_CALLBACK(thisclass, func, param, var) PacketDispatcherCallback<thisclass, param, PacketCallbackSource::Server> var; bool func(param* pParam)

class PacketDispatcher
{
	MEMORY_ALLOCATOR(PacketDispatcher, "Network");

private:
	static std::vector<PacketDispatcherCallbackBase*> m_callbacks;
	static Mutex* m_mutex;

public:
	static void Register(PacketDispatcherCallbackBase* callback);
	static void Unregister(PacketDispatcherCallbackBase* callback);

	static bool Dispatch(NetPacket* packet, bool internal_only = false);

	static void Init();

};

class PacketDispatcherCallbackBase
{
	MEMORY_ALLOCATOR(PacketDispatcherCallbackBase, "Network");

protected:
	friend class PacketDispatcher;

	virtual bool Dispatch(NetPacket* packet) = 0;

	virtual bool Is_Internal() = 0;
	virtual NetConnection* Get_Connection() = 0;

public:
	PacketDispatcherCallbackBase()
	{
		PacketDispatcher::Register(this);
	}

	virtual ~PacketDispatcherCallbackBase()
	{
		PacketDispatcher::Unregister(this);
	}
};

template <typename thisclass, typename packetclass, PacketCallbackSource::Type source>
class PacketDispatcherCallback : public PacketDispatcherCallbackBase
{
	MEMORY_ALLOCATOR(PacketDispatcherCallback, "Network");

public:
	typedef bool (thisclass::*FunctionPtr)(packetclass* packet);

	FunctionPtr					m_callback;
	thisclass*					m_instance;

	bool						m_internal;
	NetConnection*				m_only_connection;

	PacketCallbackSource::Type	m_source;

public:
	PacketDispatcherCallback(thisclass* instance, FunctionPtr callback, bool is_internal = false, NetConnection* only_connection = NULL)
		: m_callback(callback)
		, m_instance(instance)
		, m_internal(is_internal)
		, m_only_connection(only_connection)
		, m_source(source)
	{
	}

	bool Dispatch(NetPacket* packet)
	{
		packetclass* resolved = dynamic_cast<packetclass*>(packet);
		if (resolved != NULL)
		{
			if ((m_source == PacketCallbackSource::Anything) ||
				(m_source == PacketCallbackSource::Client && packet->Is_Server_Packet()) || // Client side, aka from server.
				(m_source == PacketCallbackSource::Server && !packet->Is_Server_Packet()))  // same! Nice and confusing lol.
			{
				return (m_instance->*m_callback)(resolved);
			}
		}
		return false;
	}

	bool Is_Internal() 
	{
		return m_internal;
	}

	NetConnection* Get_Connection()
	{
		return m_only_connection;
	}

};

#endif