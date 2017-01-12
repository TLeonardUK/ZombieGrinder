// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/IO/Win32/Win32_FileStreamFactory.h"
#include "Engine/IO/Win32/Win32_FileStream.h"

#include <string>
#include <Windows.h>

Stream*	Win32_FileStreamFactory::Try_Open(const char* url, StreamMode::Type mode)
{
	// See if file exists.
	FILE* exists_handle = fopen(url, "r");

	bool exists = (exists_handle != NULL);

	if (exists_handle != NULL)
	{
		fclose(exists_handle);
	}

	// Work out mode string to use.
	std::string mode_string = "";
	if ((mode & StreamMode::Write) != 0 && (mode & StreamMode::Read) != 0)
	{
		if (exists == true)
		{
			mode_string += "r+";
		}
		else
		{
			mode_string += "w+";
		}
	}
	else
	{
		if ((mode & StreamMode::Read) != 0)
		{
			mode_string += "r";

			if (exists == false)
			{
				return NULL;
			}
		}
		if ((mode & StreamMode::Write) != 0)
		{
			mode_string += "w";		
		}
	}
	if ((mode & StreamMode::Truncate) != 0)
	{
		// We don't do shit for this on windows.
	}
	if ((mode & StreamMode::Create) != 0)
	{
		// We don't do shit for this on windows.
	}
	mode_string += "b";

	FILE* handle = fopen(url, mode_string.c_str());
	if (handle == NULL)
	{
		DBG_LOG("Failed to open '%s' with mode '%s'", url, mode_string.c_str());
		return NULL;
	}

	return new Win32_FileStream(handle);
}

u64 Win32_FileStreamFactory::Try_Get_Last_Modified(const char* url)
{
	HANDLE handle = CreateFileA(url, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	FILETIME write_time;
	BOOL result = GetFileTime(handle, NULL, NULL, &write_time);

	CloseHandle(handle);

	if (result != 0)
	{
		return static_cast<u64>(write_time.dwHighDateTime) << 32 | write_time.dwLowDateTime;
	}

	return 0;
}

#endif