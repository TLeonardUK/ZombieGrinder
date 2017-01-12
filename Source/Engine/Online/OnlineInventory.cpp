// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineInventory.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineInventory.h"
#endif

OnlineInventory* OnlineInventory::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_OnlineInventory();
#else
#error "Online platform unsupported."
#endif
}

