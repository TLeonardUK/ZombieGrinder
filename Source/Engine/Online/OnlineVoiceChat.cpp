// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineVoiceChat.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineVoiceChat.h"
#endif

OnlineVoiceChat* OnlineVoiceChat::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	OnlineVoiceChat* chat = new Steamworks_OnlineVoiceChat();
	return chat;
#else
	#error "Online platform unsupported."
#endif
}

