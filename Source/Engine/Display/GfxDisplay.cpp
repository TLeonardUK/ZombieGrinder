// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Display/GfxDisplay.h"

#include "Generic/Patterns/Singleton.h"

#include "Engine/Display/Null/Null_GfxDisplay.h"

#if defined(PLATFORM_WIN32)
#include "Engine/Display/Win32/Win32_GfxDisplay.h"
#elif defined(PLATFORM_LINUX)
#include "Engine/Display/Linux/Linux_GfxDisplay.h"
#elif defined(PLATFORM_MACOS)
#include "Engine/Display/MacOS/MacOS_GfxDisplay.h"
#endif
 
#include "Engine/Engine/EngineOptions.h"

GfxDisplay* GfxDisplay::Create(const char* title, int width, int height, int hertz, GfxDisplayMode::Type fullscreen)
{
	// If we are not running with GUI we just return a
	// dummny renderer.
	if (*EngineOptions::nogui)
	{
		return new Null_GfxDisplay(title, width, height, hertz, fullscreen);
	}

#if defined(PLATFORM_WIN32)
	return new Win32_GfxDisplay(title, width, height, hertz, fullscreen);
#elif defined(PLATFORM_LINUX)
	return new Linux_GfxDisplay(title, width, height, hertz, fullscreen);
#elif defined(PLATFORM_MACOS)
	return new MacOS_GfxDisplay(title, width, height, hertz, fullscreen);
#else
	#error "Platform unsupported."
#endif
}

