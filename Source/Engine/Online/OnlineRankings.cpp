// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineRankings.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineRankings.h"
#endif

OnlineRankings* OnlineRankings::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_OnlineRankings();
#else
	#error "Online platform unsupported."
#endif
}

