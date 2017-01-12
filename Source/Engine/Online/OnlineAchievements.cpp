// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineAchievements.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineAchievements.h"
#endif

OnlineAchievements* OnlineAchievements::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_OnlineAchievements();
#else
	#error "Online platform unsupported."
#endif
}

