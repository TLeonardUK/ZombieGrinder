// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_VOICE_CHAT_
#define _ENGINE_ONLINE_VOICE_CHAT_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

#include <string>
#include <vector>

class OnlineUser;

class OnlineVoiceChat : public Singleton<OnlineVoiceChat>
{
	MEMORY_ALLOCATOR(OnlineVoiceChat, "Network");

private:

public:	
	virtual ~OnlineVoiceChat() {} 

	static OnlineVoiceChat* Create();

	virtual bool Initialize() = 0;

	virtual void Tick(const FrameTime& time) = 0;

	virtual void Set_Disabled(bool bDisabled) = 0;
	virtual bool Get_Disabled() = 0;

	virtual void Recieve_Voice_Data(OnlineUser* user, const char* data, int data_size, float output_volume) = 0;

};

#endif

