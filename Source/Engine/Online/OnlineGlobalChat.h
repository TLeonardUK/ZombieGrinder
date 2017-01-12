// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_GLOBAL_CHAT_
#define _ENGINE_ONLINE_GLOBAL_CHAT_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class OnlineUser;

struct GlobalChatMessageType
{
	enum Type
	{
		User_Joined,
		User_Left,
		Chat_Message,
	};
};

struct GlobalChatMessage
{
	GlobalChatMessageType::Type Type;
	std::string					Source;
	std::string					Message;
	int							Counter;
};

class OnlineGlobalChat : public Singleton<OnlineGlobalChat>
{
	MEMORY_ALLOCATOR(OnlineGlobalChat, "Network");

private:

public:	
	virtual ~OnlineGlobalChat() {} 

	static OnlineGlobalChat* Create();

	virtual bool Initialize() = 0;

	virtual void Tick(const FrameTime& time) = 0;

	virtual int Get_Messages(std::vector<GlobalChatMessage>& messages) = 0;

	virtual void Send_Message(GlobalChatMessage Message) = 0;

	virtual int Get_User_Count() = 0;

};

#endif

