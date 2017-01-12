// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/OnlineMods.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlineMods.h"
#endif

OnlineMods* OnlineMods::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_OnlineMods();
#else
#error "Online platform unsupported."
#endif
}
