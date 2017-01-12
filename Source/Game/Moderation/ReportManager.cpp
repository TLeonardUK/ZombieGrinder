// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game//Moderation/ReportManager.h"

#ifdef OPT_STEAM_PLATFORM
#include "Game/Moderation/Steamworks/Steamworks_ReportManager.h"
#endif

ReportManager* ReportManager::Create()
{
#if defined(OPT_STEAM_PLATFORM)
	return new Steamworks_ReportManager();
#else
#error "Online report manager unsupported."
#endif
}

