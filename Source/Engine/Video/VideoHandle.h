// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LOCALISE_VIDEOHANDLE_
#define _ENGINE_LOCALISE_VIDEOHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Localise/Language.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

// The video handle wraps a audio bank class instance, and automatically
// reloads the audio bank if the source file is changed.

class VideoFactory;
class Video;

class VideoHandle : public Reloadable
{
	MEMORY_ALLOCATOR(VideoHandle, "Video");

private:

	Video*			m_bank;
	std::string			m_url;

protected:

	friend class VideoFactory;

	// Only texture factory should be able to modify these!
	VideoHandle(const char* url, Video* sound);
	~VideoHandle();

	void Reload();

public:

	// Get/Set
	Video* Get();
	std::string Get_URL();

};

#endif

