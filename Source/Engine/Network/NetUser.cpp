// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Network/NetUser.h"
#include "Engine/Network/NetConnection.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Online/OnlineServer.h"

NetUser::NetUser(NetConnection* connection, int local_user_index, OnlineUser* online_user)
	: m_connection(connection)
	, m_local_user_index(local_user_index)
	, m_online_user(online_user)
	, m_username("")
	, m_score(0)	
{
}

NetUser::~NetUser()
{
	// Do not delete connections, ownership is part of NetManager.
	if (m_connection != NULL && m_local_user_index == 0)
	{
		m_connection->Dispose();
		m_connection = NULL;
	}
}

void NetUser::Reset_On_Map_Load()
{
}

int	NetUser::Get_Local_User_Index()
{
	return m_local_user_index;
}

NetConnection* NetUser::Get_Connection()
{
	return m_connection;
}

OnlineUser* NetUser::Get_Online_User()
{
	return m_online_user;
}

std::string NetUser::Get_Username()
{
	return m_username;
}

std::string NetUser::Get_Display_Username(std::string username)
{
	return username == "" ? m_username : username;
}

u16 NetUser::Get_Net_ID()
{
	return m_net_id;
}

void NetUser::Set_Username(std::string username)
{
	m_username = username;
}

void NetUser::Set_Score(int score)
{
	if (m_score != score)
	{
		OnlineServer* server = OnlinePlatform::Get()->Server();
		if (server != NULL)
			server->Flag_Server_Info_Dirty();
	}
	m_score = score;
}

int	NetUser::Get_Score()
{
	return m_score;
}

void NetUser::Set_Net_ID(u16 id)
{
	m_net_id = id;
}

void NetUser::Tick(const FrameTime& time)
{
	// Dodedodododo
}