// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_CLIENT_
#define _ENGINE_STEAMWORKS_ONLINE_CLIENT_

#include "Engine/Online/OnlineClient.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

#include "Engine/Network/NetClient.h"

class Steamworks_OnlineClient : public OnlineClient
{
	MEMORY_ALLOCATOR(Steamworks_OnlineClient, "Network");

private:
	Steamworks_OnlinePlatform* m_platform;
	bool m_initialised;
	
	NetClient_State::Type m_old_client_state;

protected:
	void Update_Presence();

public:
	Steamworks_OnlineClient(Steamworks_OnlinePlatform* platform);
	~Steamworks_OnlineClient();

	bool Init();

	void Tick(const FrameTime& time);

	void Set_Notification_Corner(int corner);

	void Show_Invite_Dialog();
	void Show_Profile_Dialog(OnlineUser* user);

};

#endif

