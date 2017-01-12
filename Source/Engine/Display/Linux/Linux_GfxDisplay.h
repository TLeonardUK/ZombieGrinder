// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _ENGINE_LINUX_DISPLAY_
#define _ENGINE_LINUX_DISPLAY_

#include "Engine/Display/GfxDisplay.h"

#define KEY_COUNT 256
#define MAX_TITLE_LENGTH 256

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>                                                             
#include <X11/keysym.h>    
#include <X11/extensions/xf86vmode.h>    

#include <GL/gl.h>           
#include <GL/glx.h>         

class Linux_MouseState;

class Linux_GfxDisplay : public GfxDisplay
{
	MEMORY_ALLOCATOR(Linux_GfxDisplay, "Display");

private:
	friend class Linux_MouseState;
	friend class Linux_KeyboardState;

	char					m_title[MAX_TITLE_LENGTH];
	int						m_width;
	int						m_height;
	GfxDisplayMode::Type	m_mode;
	int						m_hertz;
	bool					m_active;

	Display*				m_display;
	Window					m_root_window;
	Window					m_window;
	Colormap				m_color_map;
	XSetWindowAttributes    m_window_attr;
	GLXContext				m_context;
	int						m_default_screen;
	XF86VidModeModeInfo		m_desktop_mode;

	int*					m_icon_data;
	int						m_icon_data_len;
	
	Cursor					m_blank_cursor;

	int						m_display_width;
	int						m_display_height;

	XIM 					m_inputMethod;
	XIC 					m_inputContext;

private:
	void Setup_Window();
	void Destroy_Window();
	void Change_Resolution(XF86VidModeModeInfo* mode, bool desktop_size = false);
	void Set_Window_Bordered(bool bordered);
	void Set_Window_Location(int x, int y, int w, int h);
	void Set_Window_Input(bool exclusive);
	void Set_Window_Properties(const char* title, int min_w, int min_h);	
	void Set_Window_Icon(int* data, int len);

public:

	// Constructors.
	Linux_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode);
	~Linux_GfxDisplay();

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

#endif