// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Renderer.h"

#include "Generic/Patterns/Singleton.h"
#include "Generic/Math/Math.h"

#include "Engine/Renderer/Null/Null_Renderer.h"

#ifndef DEDICATED_SERVER_BUILD
#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include "Engine/Renderer/OpenGL/OpenGL_Renderer.h"
#endif
#endif

#include "Engine/Engine/EngineOptions.h"

Renderer* Renderer::Create()
{
#ifdef DEDICATED_SERVER_BUILD
	return new Null_Renderer();
#else
	// If we are not running with GUI we just return a
	// dummny renderer.
	if (*EngineOptions::nogui)
	{
		return new Null_Renderer();
	}

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
	return new OpenGL_Renderer();
#else
	#error "Platform unsupported."
#endif
#endif
}
