// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_WIN32_DISPLAY_
#define _ENGINE_WIN32_DISPLAY_

#include "Engine/Display/GfxDisplay.h"

#include <Windows.h>
#include <gl\GL.h>
#include <gl\GLu.h>
#include <gl\glext.h>

#define KEY_COUNT 256
#define MAX_TITLE_LENGTH 256

class Win32_GfxDisplay : public GfxDisplay
{
	MEMORY_ALLOCATOR(Win32_GfxDisplay, "GfxDisplay");

private:
	char					m_title[MAX_TITLE_LENGTH];
	int						m_width;
	int						m_height;
	GfxDisplayMode::Type	m_mode;
	int						m_hertz;
	bool					m_active;
	
	bool					m_display_changed;

#ifdef UNICODE 
	LPCWSTR		m_class_name;
#else
	LPCSTR		m_class_name;
#endif
	HDC			m_device_context;
	HGLRC		m_render_context;
	HWND		m_window_handle;
	HINSTANCE	m_instance_handle;

	HDEVNOTIFY	m_device_change_notification;
	bool		m_device_change_registered;

	void Setup_Window();
	void Destroy_Window();
	void Update_Console();

	void Register_For_Device_Change(HWND hwnd);
	void Unregister_For_Device_Change();

public:

	// Constructors.
	Win32_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode);
	~Win32_GfxDisplay();

	// Win32 specific.
	HWND Get_Window_Handle();
	
	// Base functions.	
	void Tick(const FrameTime& time);
	void Swap_Buffers();

	// Event handling.
	static LRESULT CALLBACK Static_Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
	LRESULT CALLBACK Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

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
	bool  Is_Active();
	
	// General stuff.
	std::vector<GfxDisplayResolution> Get_Available_Resolutions();

};

#endif

#endif
