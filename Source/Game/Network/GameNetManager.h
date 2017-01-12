// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_GAME_NET_MANAGER_
#define _GAME_NETWORK_GAME_NET_MANAGER_

#include "Engine/Network/NetManager.h"

class GameNetClient;
class GameNetServer;
class GameNetUser;
class ChatManager;
class EditorManager;
class GameNetServerState;

class GameNetManager : public NetManager
{
	MEMORY_ALLOCATOR(GameNetManager, "Network");

private:
	GameNetClient* m_client;
	GameNetServer* m_server;

public:
	static GameNetManager* Get()
	{
		return static_cast<GameNetManager*>(NetManager::Get());
	}

	GameNetManager();
	~GameNetManager();

	bool Init();
	void DeInit();
	
	bool Init_Client();
	bool Init_Server();
	
	bool Destroy_Client();
	bool Destroy_Server();

	bool Is_Game_Active();

	void Update_User_Scoreboard_Ranks();

	NetClient* Client();
	NetServer* Server();

	GameNetClient* Game_Client();
	GameNetServer* Game_Server();
	
	unsigned int Get_Net_Version();
	
	GameNetUser* Get_Game_Net_User_By_Connection(NetConnection* connection);
	std::vector<GameNetUser*> Get_Game_Net_Users();

	GameNetUser* Get_Game_User_By_Net_ID(int net_id);
	
	void Leave_Network_Game();

	void Editor_Mode_Changed();

	GameNetServerState& Get_Server_State();

	bool Any_Users_In_Cutscene();

	void Tick(const FrameTime& time);

	NetUser* Create_Net_User(NetConnection* connection, int local_user_index, OnlineUser* user = NULL);

};

#endif

