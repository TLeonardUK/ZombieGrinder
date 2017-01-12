// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NET_PACKET_
#define _ENGINE_NET_PACKET_

#include "Generic/Types/HashTable.h"

class Stream;
class NetPacket;
class NetConnection;

typedef NetPacket* (*NetPacket_Constructor_Function) ();

class NetPacket
{
	MEMORY_ALLOCATOR(NetPacket, "Network");

private:
	NetConnection* m_recieved_from;
	bool m_is_server_packet;

protected:
	static HashTable<NetPacket_Constructor_Function, unsigned int> m_packet_constructors;

public:
	NetPacket();
	virtual ~NetPacket();

	static NetPacket* Create_By_Type_ID(unsigned int id);
	static void Register_Type_ID(unsigned int id, NetPacket_Constructor_Function constructor);

	void Set_Recieved_From(NetConnection* from);
	NetConnection* Get_Recieved_From();

	bool Is_Server_Packet();
	void Set_Server_Packet(bool value);

	virtual const char* Get_Name();

	virtual unsigned int Get_Type_ID() = 0;

	virtual void Serialize(Stream* stream) = 0;
	virtual bool Deserialize(Stream* stream) = 0;

	virtual bool Is_OutOfBand() = 0;

};

#endif

