// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NULL_DISPLAY_
#define _ENGINE_NULL_DISPLAY_

#include "Engine/Display/GfxDisplay.h"

#define KEY_COUNT 256
#define MAX_TITLE_LENGTH 256

class Null_GfxDisplay : public GfxDisplay
{
	MEMORY_ALLOCATOR(Null_GfxDisplay, "Display");

private:
	char					m_title[MAX_TITLE_LENGTH];
	int						m_width;
	int						m_height;
	GfxDisplayMode::Type	m_mode;
	int						m_hertz;
	bool					m_active;

public:

	// Constructors.
	Null_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode);
	~Null_GfxDisplay();

	// Base functions.	
	void Tick(const FrameTime& time);
	void Swap_Buffers();

	// Properties
	const char* Get_Title();
	int Get_Width();
	int Get_Height();
	int Get_Hertz();
	GfxDisplayMode::Type Get_Mode();

	// Modifiers
	void Set_Title(const char* title);
	void Set_Hidden(bool bHidden);
	bool Resize(int width, int height, int hertz, GfxDisplayMode::Type mode);

	// Input.
	bool Is_Active();
	
	// General stuff.
	std::vector<GfxDisplayResolution> Get_Available_Resolutions();

};

#endif

