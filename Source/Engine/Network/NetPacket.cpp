// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetPacket.h"

HashTable<NetPacket_Constructor_Function, unsigned int> NetPacket::m_packet_constructors;

NetPacket::NetPacket()
	: m_recieved_from(NULL)
	, m_is_server_packet(false)
{
}

NetPacket::~NetPacket()
{
}

const char* NetPacket::Get_Name()
{
	return "unknown";
}

NetPacket* NetPacket::Create_By_Type_ID(unsigned int id)
{
	NetPacket_Constructor_Function func = NULL;
	
	if (m_packet_constructors.Get(id, func))
	{
		return func();
	}

	return NULL;
}

void NetPacket::Register_Type_ID(unsigned int id, NetPacket_Constructor_Function constructor)
{
	m_packet_constructors.Set(id, constructor);
}

void NetPacket::Set_Recieved_From(NetConnection* from)
{
	m_recieved_from = from;
}

NetConnection* NetPacket::Get_Recieved_From()
{
	return m_recieved_from;
}

bool NetPacket::Is_Server_Packet()
{
	return m_is_server_packet;
}

void NetPacket::Set_Server_Packet(bool value)
{
	m_is_server_packet = value;
}
