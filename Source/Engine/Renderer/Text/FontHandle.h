// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FONTHANDLE_
#define _ENGINE_RENDERER_TEXT_FONTHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Text/Font.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

// The sound handle wraps a Font class instance, and automatically
// reloads the font if the source file is changed.

class FontFactory;
class Font;

class FontHandle : public Reloadable
{
	MEMORY_ALLOCATOR(FontHandle, "Rendering");

private:

	Font*				m_font;
	std::string			m_url;
	FontFlags::Type		m_flags;

protected:
	
	friend class FontFactory;

	// Only font factory should be able to modify these!
	FontHandle(const char* url, FontFlags::Type flags, Font* font);
	~FontHandle();

	void Reload();

public:

	// Get/Set
	Font* Get();
	std::string Get_URL();

};

#endif

