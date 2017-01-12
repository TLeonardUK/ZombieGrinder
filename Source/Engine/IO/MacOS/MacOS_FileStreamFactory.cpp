// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/IO/MacOS/MacOS_FileStreamFactory.h"
#include "Engine/IO/MacOS/MacOS_FileStream.h"

#include <string>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

Stream*	MacOS_FileStreamFactory::Try_Open(const char* url, StreamMode::Type mode)
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
		return NULL;
	}

	return new MacOS_FileStream(handle);
}

u64 MacOS_FileStreamFactory::Try_Get_Last_Modified(const char* url)
{
	int fd = open(url, O_RDWR);
	if (fd <= 0)
	{
		return 0;
	}

	struct stat stats;
	fstat(fd, &stats);

	close(fd);

	return stats.st_ctime;
}

#endif