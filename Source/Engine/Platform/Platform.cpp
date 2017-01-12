// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Platform/Platform.h"

#include "Generic/Patterns/Singleton.h"

#if defined(PLATFORM_WIN32)
#include "Engine/Platform/Win32/Win32_Platform.h"
#elif defined(PLATFORM_LINUX)
#include "Engine/Platform/Linux/Linux_Platform.h"
#elif defined(PLATFORM_MACOS)
#include "Engine/Platform/MacOS/MacOS_Platform.h"
#endif

Platform* Platform::Create()
{
#if defined(PLATFORM_WIN32)
	return new Win32_Platform();
#elif defined(PLATFORM_LINUX)
	return new Linux_Platform();
#elif defined(PLATFORM_MACOS)
	return new MacOS_Platform();
#else
	#error "Platform unsupported."
#endif
}

std::string Platform::Change_Filename(std::string original, std::string new_filename)
{
	return Extract_Directory(original) + "/" + new_filename;
}

std::string Platform::Change_Extension(std::string original, std::string new_filename)
{
	return Extract_Directory(original) + "/" + Extract_Basename(original) + new_filename;
}
