// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineGlobalChat.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineGlobalChat.h"
#endif

OnlineGlobalChat* OnlineGlobalChat::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_OnlineGlobalChat();
#else
#error "Online platform unsupported."
#endif
}

