// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Storage/PersistentStorage.h"

#include "Generic/Patterns/Singleton.h"

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include "Engine/Storage/Steamworks/Steamworks_PersistentStorage.h"
#endif

PersistentStorage* PersistentStorage::Create()
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
	return new Steamworks_PersistentStorage();
#else
	#error "Platform unsupported."
#endif
}

