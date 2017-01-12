// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LOCALISE_VIDEO_
#define _ENGINE_LOCALISE_VIDEO_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/ThreadSafeHashTable.h"
#include "Generic/Patterns/Singleton.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

class Video
{
	MEMORY_ALLOCATOR(Video, "Engine");

private:
	friend class VideoFactory;

	std::string m_url;
	std::string m_subtitle_url;

public:

	std::string Get_URL() 
	{
		return m_url;
	}

	std::string Get_Subtitle_URL()
	{
		return m_subtitle_url;
	}

public:
	Video();
	~Video();

};

#endif

