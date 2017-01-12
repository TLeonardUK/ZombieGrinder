// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_GLOBAL_CHAT_
#define _ENGINE_STEAMWORKS_ONLINE_GLOBAL_CHAT_

#include "Engine/Online/OnlineGlobalChat.h"
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

#include "public/steam/steam_api.h"

struct Steamworks_OnlineGlobalChatState
{
	enum Type
	{
		Offline,
		Connecting,
		Online,
		Banned,
	};
};

class Steamworks_OnlineGlobalChat : public OnlineGlobalChat
{
	MEMORY_ALLOCATOR(Steamworks_OnlineGlobalChat, "Network");

private:
	Steamworks_OnlineGlobalChatState::Type m_state;

	SteamAPICall_t m_join_request;	
	CSteamID m_lobby_id;
	CCallResult<Steamworks_OnlineGlobalChat, JoinClanChatRoomCompletionResult_t> m_join_request_result;

	std::vector<GlobalChatMessage> m_messages;
	int m_message_recieve_count;

	enum 
	{
		max_messages = 30,
	};

private:
	STEAM_CALLBACK(Steamworks_OnlineGlobalChat, Callback_On_User_Left, GameConnectedChatLeave_t, m_on_user_left_callback);
	STEAM_CALLBACK(Steamworks_OnlineGlobalChat, Callback_On_Recieved_Message, GameConnectedClanChatMsg_t, m_on_recieved_message_callback);	

	void On_Join_Finished(JoinClanChatRoomCompletionResult_t* params, bool bIOFailure);

	void Store_Message(GlobalChatMessage Message);

public:
	Steamworks_OnlineGlobalChat();
	~Steamworks_OnlineGlobalChat();

	bool Initialize();

	void Tick(const FrameTime& time);

	int Get_Messages(std::vector<GlobalChatMessage>& messages);

	void Send_Message(GlobalChatMessage Message);

	int Get_User_Count();

};

#endif

