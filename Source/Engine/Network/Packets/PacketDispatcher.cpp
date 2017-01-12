// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/Packets/PacketDispatcher.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include <algorithm>

std::vector<PacketDispatcherCallbackBase*> PacketDispatcher::m_callbacks;
Mutex* PacketDispatcher::m_mutex = NULL;

void PacketDispatcher::Init()
{
	m_mutex = Mutex::Create();
}

void PacketDispatcher::Register(PacketDispatcherCallbackBase* callback)
{
	MutexLock lock(m_mutex);

	m_callbacks.push_back(callback);
}

void PacketDispatcher::Unregister(PacketDispatcherCallbackBase* callback)
{
	MutexLock lock(m_mutex);

	std::vector<PacketDispatcherCallbackBase*>::iterator iter = std::find(m_callbacks.begin(), m_callbacks.end(), callback);
	if (iter != m_callbacks.end())
	{
		m_callbacks.erase(iter);
	}
}

bool PacketDispatcher::Dispatch(NetPacket* packet, bool internal_only)
{
	MutexLock lock(m_mutex);

	for (std::vector<PacketDispatcherCallbackBase*>::iterator iter = m_callbacks.begin(); iter != m_callbacks.end(); iter++)
	{
		PacketDispatcherCallbackBase* base = *iter;

		if (internal_only == true && base->Is_Internal() == false)
		{
			continue;
		}

		if (base->Get_Connection() != NULL && base->Get_Connection() != packet->Get_Recieved_From())
		{
			continue;
		}

		if (base->Dispatch(packet))
		{
			return true;
		}
	}

	return false;
}
