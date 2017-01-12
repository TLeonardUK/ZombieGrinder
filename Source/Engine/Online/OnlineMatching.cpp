// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineMatching.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#endif

OnlineMatching* OnlineMatching::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_OnlineMatching();
#else
	#error "Online platform unsupported."
#endif
}

