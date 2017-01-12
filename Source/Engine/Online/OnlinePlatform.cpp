// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlinePlatform.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#endif

OnlinePlatform* OnlinePlatform::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_OnlinePlatform();
#else
	#error "Online platform unsupported."
#endif
}

