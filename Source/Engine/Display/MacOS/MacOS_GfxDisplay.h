// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_MACOS_DISPLAY_
#define _ENGINE_MACOS_DISPLAY_

#include "Engine/Display/GfxDisplay.h"

#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h>

#define KEY_COUNT 256
#define MAX_TITLE_LENGTH 256

class MacOS_GfxDisplay : public GfxDisplay
{
	MEMORY_ALLOCATOR(MacOS_GfxDisplay, "GfxDisplay");

private:
	char					m_title[MAX_TITLE_LENGTH];

	int						m_width;
	int						m_height;
	GfxDisplayMode::Type	m_mode;
	int						m_hertz;

	int						m_old_width;
	int						m_old_height;
	GfxDisplayMode::Type	m_old_mode;
	int						m_old_hertz;

	bool					m_active;

	// Would be nicer if I didn't have to use voids ;_;
	void*					m_ns_window;
	void*					m_ns_gl_context;

	int						m_original_resolution_width;
	int						m_original_resolution_height;

	int						m_window_area_x;
	int						m_window_area_y;
	int						m_window_area_width;
	int						m_window_area_height;

	bool					m_resolution_changed;

	bool					m_is_display_captured;

	int						m_fullscreen_depth;

	void Setup_Window();
	void Destroy_Window();

	bool Change_Resolution(int width, int height);
	void Reset_Resolution();

	void Calculate_Window_Area();

public:
	void Handle_Event(void* evt_pimpl, void* view_pimpl);

public:

	// Constructors.
	MacOS_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode);
	~MacOS_GfxDisplay();

	// Base functions.	
	void Tick(const FrameTime& time);
	void Swap_Buffers();

	void Set_VSync(bool enabled);

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

#endif