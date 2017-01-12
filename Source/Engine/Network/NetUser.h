// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NET_USER_
#define _ENGINE_NET_USER_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Network/Packets/EnginePackets.h"
#include "Engine/Network/Packets/PacketDispatcher.h"

#include <string>

class NetConnection;
class OnlineUser;
class NetManager;

class NetUser
{
	MEMORY_ALLOCATOR(NetUser, "Network");

private:
	
protected:
	friend class NetManager;

	NetConnection* m_connection;
	int m_local_user_index;

	OnlineUser* m_online_user;

	int m_score;
	std::string m_username;
	
	u16 m_net_id;

public:
	virtual ~NetUser();
	NetUser(NetConnection* connection, int local_user_index, OnlineUser* online_user);

	NetConnection*		Get_Connection();
	int					Get_Local_User_Index();
	OnlineUser*			Get_Online_User();
	u16					Get_Net_ID();

	virtual std::string Get_Display_Username(std::string username = "");
	virtual std::string Get_Username();
	virtual void		Set_Username(std::string username);
	virtual void		Set_Score(int score);
	virtual int			Get_Score();

	virtual void		Reset_On_Map_Load();

	void				Set_Net_ID(u16 id);

	virtual void Tick(const FrameTime& time);

};

#endif

