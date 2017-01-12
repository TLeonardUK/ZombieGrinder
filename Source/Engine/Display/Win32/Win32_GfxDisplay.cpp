// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/Display/Win32/Win32_GfxDisplay.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/Input/Win32/Win32_KeyboardState.h"
#include "Engine/Input/Win32/Win32_MouseState.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Engine/Engine/EngineOptions.h"

#include <string>
#include <dbt.h>
#include <windows.h>

Win32_GfxDisplay::Win32_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode)
	: m_width(width)
	, m_height(height)
	, m_mode(mode)
	, m_hertz(hertz)
	, m_class_name(NULL)
	, m_device_context(NULL)
	, m_render_context(NULL)
	, m_window_handle(NULL)
	, m_instance_handle(NULL)
	, m_active(false)
	, m_display_changed(false)
	, m_device_change_registered(false)
{	
	memset(m_title, 0, MAX_TITLE_LENGTH);
	strcpy(m_title, title);

	Setup_Window();
}

HWND Win32_GfxDisplay::Get_Window_Handle()
{
	return m_window_handle;
}

Win32_GfxDisplay::~Win32_GfxDisplay()
{
	Unregister_For_Device_Change();
	Destroy_Window();
}

const char* Win32_GfxDisplay::Get_Title()
{
	return m_title;
}

int Win32_GfxDisplay::Get_Width()
{
	return m_width;
}

int Win32_GfxDisplay::Get_Height()
{
	return m_height;
}

int Win32_GfxDisplay::Get_Hertz()
{
	return m_hertz;
}

GfxDisplayMode::Type Win32_GfxDisplay::Get_Mode()
{
	return m_mode;
}

bool Win32_GfxDisplay::Is_Active()
{
	return m_active;
}

void Win32_GfxDisplay::Set_Title(const char* title)
{
	if (strcmp(m_title, title) == 0)
	{
		return;
	}

	strcpy(m_title, title);
	if (m_window_handle)
	{
		SetWindowTextA(m_window_handle, m_title);
		SetConsoleTitleA(m_title);
	}
}

void Win32_GfxDisplay::Set_Hidden(bool bHidden)
{
	if (m_window_handle)
	{
		ShowWindow(m_window_handle, bHidden ? SW_HIDE : SW_SHOW);
	}

	if (m_display_changed)
	{
		ChangeDisplaySettings(NULL, 0);
		m_display_changed = false;
	}
}

bool Win32_GfxDisplay::Resize(int width, int height, int hertz, GfxDisplayMode::Type mode)
{
	if (m_width		== width &&
		m_height	== height &&
		m_hertz		== hertz &&
		m_mode		== mode)
	{
		return false;
	}

	m_width = width;
	m_height = height;
	m_hertz = hertz;
	m_mode = mode;

	// Change window position/size
	RECT window_rect;
	memset(&window_rect, 0, sizeof(window_rect));

	int screen_width;
	int	screen_height;

	DWORD dw_extented_style;
	DWORD dw_style;
	
	dw_extented_style	= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dw_style			= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	switch (m_mode)
	{
	case GfxDisplayMode::Fullscreen:
		{
			window_rect.left	= 0;
			window_rect.top		= 0;
			window_rect.right	= window_rect.left + m_width;
			window_rect.bottom	= window_rect.top + m_height;

			// Change graphics mode to fullscreen.
			DEVMODE dmScreenSettings;                  
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));      

			dmScreenSettings.dmSize				= sizeof(dmScreenSettings);      
			dmScreenSettings.dmPelsWidth		= m_width;            
			dmScreenSettings.dmPelsHeight		= m_height;       
			dmScreenSettings.dmDisplayFrequency	= m_hertz;
			dmScreenSettings.dmBitsPerPel		= 32;            
			dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		
			dw_extented_style	= WS_EX_APPWINDOW; 
			dw_style			= WS_POPUP;

			if (m_display_changed)
			{
				ChangeDisplaySettings(NULL, 0);
				m_display_changed = false;
			}

			int ret = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
			if (ret == DISP_CHANGE_SUCCESSFUL)
			{
				dw_extented_style = WS_EX_APPWINDOW;
				dw_style = WS_POPUP;
				m_display_changed = true;
				break;
			}
			else
			{
				// Fallthrough to windowed fullscreen if we couldn't change display settings.
			}
		}
	case GfxDisplayMode::FullscreenWindowed:
		{
			if (m_display_changed)
			{
				ChangeDisplaySettings(NULL, 0);
				m_display_changed = false;
			}

			screen_width		= GetSystemMetrics(SM_CXSCREEN);
			screen_height		= GetSystemMetrics(SM_CYSCREEN);

			window_rect.left	= 0;
			window_rect.top		= 0;
			window_rect.right	= window_rect.left + screen_width;
			window_rect.bottom	= window_rect.top + screen_height;

			dw_extented_style	= WS_EX_APPWINDOW; 
			dw_style			= WS_POPUP;

			m_width				= screen_width;
			m_height			= screen_height;

			break;
		}
	case GfxDisplayMode::Windowed:
		{
			if (m_display_changed)
			{
				ChangeDisplaySettings(NULL, 0);
				m_display_changed = false;
			}

			screen_width		= GetSystemMetrics(SM_CXSCREEN);
			screen_height		= GetSystemMetrics(SM_CYSCREEN);

			window_rect.left	= (screen_width / 2) - (m_width / 2);
			window_rect.top		= (screen_height / 2) - (m_height / 2);
			window_rect.right	= window_rect.left + m_width;
			window_rect.bottom	= window_rect.top + m_height;

			break;
		}
	}

	// Adjust window style.
	SetWindowLong(m_window_handle, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dw_style);
	SetWindowLong(m_window_handle, GWL_EXSTYLE, dw_extented_style);

	// Make sure window rect is correct.
	{
		BOOL ret = AdjustWindowRectEx(&window_rect, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dw_style, false, dw_extented_style);
		DBG_ASSERT(ret != 0);
	}

	// Adjust window position.
	SetWindowPos(m_window_handle, 0, window_rect.left, window_rect.top, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, 0);

	// Show window.
	ShowWindow(m_window_handle, SW_SHOW);
	SetForegroundWindow(m_window_handle);
	SetFocus(m_window_handle);

	// Update console.
	Update_Console();

	return true;
}

void Win32_GfxDisplay::Update_Console()
{
	if (m_mode != GfxDisplayMode::Fullscreen)
	{
		HWND console_window = GetConsoleWindow();
		if (console_window != NULL)
		{
			RECT parent_rect;
			GetWindowRect(m_window_handle, &parent_rect);
			MoveWindow(console_window, parent_rect.left, parent_rect.bottom + 5, parent_rect.right - parent_rect.left, 300, true);
		}
	}
}

#define USE_WINW_UNICODE

void Win32_GfxDisplay::Tick(const FrameTime& time)
{
	MSG message;

//#ifndef DEBUG_BUILD
//	do
//#endif
//	{
		// Pump message queue.
#ifdef USE_WINW_UNICODE
		while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
#else
		while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
#endif
		{
			if (message.message == WM_QUIT)
			{
				GameEngine::Get()->Stop();
			}
			else
			{
#ifdef USE_WINW_UNICODE
				TranslateMessage(&message);
				DispatchMessageW(&message);   
#else
				TranslateMessage(&message);
				DispatchMessage(&message);   
#endif
			}
		}

		// If we have a console window, snap to the side of this window.
		//Update_Console();

		// Sleep if not active.
//#ifndef DEBUG_BUILD
//		if (m_active == false)
//		{
//			Sleep(100);
//		}
//#endif
//	} 
//#ifndef DEBUG_BUILD
//	while (m_active == false);
//#endif
}

LRESULT CALLBACK Win32_GfxDisplay::Static_Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	Win32_GfxDisplay* display = reinterpret_cast<Win32_GfxDisplay*>(GetWindowLong(hwnd, GWLP_USERDATA));
	if (display)
	{
		return display->Event_Handler(hwnd, umsg, wparam, lparam);
	}

#ifdef USE_WINW_UNICODE
	return DefWindowProcW(hwnd, umsg, wparam, lparam);
#else
	return DefWindowProc(hwnd, umsg, wparam, lparam);
#endif
}

void Win32_GfxDisplay::Register_For_Device_Change(HWND hwnd)
{
	if (m_device_change_registered)
	{
		return;
	}

	// GUID for all serial host pnp drivers:
	//		https://msdn.microsoft.com/en-us/library/aa363432(VS.85).aspx
	GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72,0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };
	
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

	ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size		= sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype	= DBT_DEVTYP_DEVICEINTERFACE;
	NotificationFilter.dbcc_classguid	= WceusbshGUID;

	m_device_change_notification = RegisterDeviceNotification
	(
		hwnd, 
		&NotificationFilter,   
		DEVICE_NOTIFY_WINDOW_HANDLE 
	);

	m_device_change_registered = true;
}

void Win32_GfxDisplay::Unregister_For_Device_Change()
{
	if (!m_device_change_registered)
	{
		return;
	}

	UnregisterDeviceNotification(m_device_change_notification);
}

LRESULT CALLBACK Win32_GfxDisplay::Event_Handler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
/*	if (umsg != 0x007f && umsg != 0x00ae && umsg != 0x000c && umsg != 0x0020 && umsg != 0x0084)
	{
		DBG_LOG("umsg=0x%04x", umsg);
	}
*/
	switch (umsg)
	{
	case WM_DEVICECHANGE:
		{
			DBG_LOG("Recieved device change notification. Force rescann for input devices.");
			if (Input::Get() && Input::Get()->Get_Joystick_State())
			{
				Input::Get()->Global_Scan_For_Devices();
			}
			break;
		}
	case WM_CREATE:
		{
			Register_For_Device_Change(hwnd);
			break;
		}
	case WM_MOVE:
		{
			Update_Console();
			break;
		}
	case WM_ACTIVATE:
		{
			m_active = (wparam == WA_ACTIVE || wparam == WA_CLICKACTIVE);
			if (!m_active)
			{
				// Release all keys, stops us ending up with inconsistent key states due to not getting the up/down events 
				// when unfocused.
				Win32_MouseState::Clear_State();
				Win32_KeyboardState::Clear_State();
			}
			return 0;
		}
	case WM_SYSCOMMAND:
		{
			switch (wparam)
			{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				return 0;
			}
			break;
		}

	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
	/*
	case WM_IME_COMPOSITION:
		{
			if (lparam & GCS_RESULTSTR) 
			{
				HIMC hIMC = ImmGetContext(hwnd);

				DWORD dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);
				dwSize += sizeof(WCHAR);

				char* result = new char[dwSize];
				memset(result, 0, dwSize);
				ImmGetCompositionString(hIMC, GCS_RESULTSTR, result, dwSize);
				ImmReleaseContext(hwnd, hIMC);

				DBG_LOG("GOT COMPOSTION: %s", result);
				SAFE_DELETE(result);

				return 0;
			}
			break;
		}
	*/

	// TODO: UNICODE
	case WM_CHAR:
		{
			/*
#ifdef UNICODE 
			char buffer[9];
			wcstombs(buffer, (TCHAR*)&wparam, 8);
#else
			char buffer[9];
			buffer[0] = (TCHAR)wparam;
			buffer[1] = '\0';
#endif
			*/

			char out_buffer[64];
			memset(out_buffer, 0, 64);
			WCHAR wide_chr = (WCHAR)wparam;

			int bytes = WideCharToMultiByte(CP_UTF8, 0, &wide_chr, 1, out_buffer, 63, 0, NULL);
			if (bytes > 0 && bytes < 63)
			{
				out_buffer[bytes] = '\0';

				std::string chr(out_buffer);
				//DBG_LOG("Input=%s", out_buffer);

				Win32_KeyboardState::Post_Character(chr);
			}

			return 0;
		}
		
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYUP:
		{
			UINT scancode = (lparam & 0x00ff0000) >> 16;
			int extended  = (lparam & 0x01000000) != 0;

			switch (wparam) 
			{
				case VK_SHIFT:
					wparam = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
					break;
				case VK_CONTROL:
					wparam = extended ? VK_RCONTROL : VK_LCONTROL;
					break;
				case VK_MENU:
					wparam = extended ? VK_RMENU : VK_LMENU;
					break;
			}

			if (umsg == WM_SYSKEYDOWN || umsg == WM_KEYDOWN)
			{
				Win32_KeyboardState::Post_Key_Down(wparam);
			}
			else
			{				
				Win32_KeyboardState::Post_Key_Up(wparam);
			}
			return 0;
		}

	case WM_LBUTTONDOWN:
		{
			Win32_MouseState::Post_Key_Down(VK_LBUTTON);
			return 0;
		}
	case WM_MBUTTONDOWN:
		{
			Win32_MouseState::Post_Key_Down(VK_MBUTTON);
			return 0;
		}
	case WM_RBUTTONDOWN:
		{
			Win32_MouseState::Post_Key_Down(VK_RBUTTON);
			return 0;
		}

	case WM_LBUTTONUP:
		{
			Win32_MouseState::Post_Key_Up(VK_LBUTTON);
			return 0;
		}
	case WM_MBUTTONUP:
		{
			Win32_MouseState::Post_Key_Up(VK_MBUTTON);
			return 0;
		}
	case WM_RBUTTONUP:
		{
			Win32_MouseState::Post_Key_Up(VK_RBUTTON);
			return 0;
		}

	case WM_MOUSEWHEEL:
		{
			float zDelta = GET_WHEEL_DELTA_WPARAM(wparam) / 120.0f;
			Win32_MouseState::Post_Wheel_Event(zDelta);
		}
	}

#ifdef USE_WINW_UNICODE
	return DefWindowProcW(hwnd, umsg, wparam, lparam);
#else
	return DefWindowProc(hwnd, umsg, wparam, lparam);
#endif
}

void Win32_GfxDisplay::Setup_Window()
{
#ifdef USE_WINW_UNICODE
	WNDCLASSEXW	window_class;
#else
	WNDCLASSEX	window_class;
#endif

	RECT		window_rect;
	DWORD		dw_extented_style;
	DWORD		dw_style;
	int			color_depth;
	int			depth_depth;
	int			stencil_depth;
	int			screen_width;
	int			screen_height;

	screen_width		= GetSystemMetrics(SM_CXSCREEN);
	screen_height		= GetSystemMetrics(SM_CYSCREEN);

	window_rect.left	= (screen_width / 2) - (m_width / 2);
	window_rect.top		= (screen_height / 2) - (m_height / 2);
	window_rect.right	= window_rect.left + m_width;
	window_rect.bottom	= window_rect.top + m_height;

	color_depth			= 32;
	depth_depth			= 0;//16;
	stencil_depth		= 0;

	dw_extented_style	= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dw_style			= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	
#ifdef UNICODE 
	m_class_name		= L"Engine_Win32_GfxDisplay";
#else
	m_class_name		= "Engine_Win32_GfxDisplay";
#endif

	m_instance_handle = GetModuleHandle(NULL);
	DBG_ASSERT(m_instance_handle);

	// Convert title to widestring.
#ifdef UNICODE 
	int title_len = strlen(m_title);
	WCHAR* wchar_array = new WCHAR[title_len + 1];
	MultiByteToWideChar(0, 0, m_title, -1, wchar_array, title_len + 1);
	LPCWSTR wchar_title = wchar_array;
#endif

	// Setup window class.
	memset(&window_class, 0, sizeof(window_class));
#ifdef USE_WINW_UNICODE
	window_class.cbSize			= sizeof(WNDCLASSEXW);
#else
	window_class.cbSize			= sizeof(WNDCLASSEX);
#endif
	window_class.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	window_class.lpfnWndProc	= &Win32_GfxDisplay::Static_Event_Handler;
	window_class.cbClsExtra		= NULL;
	window_class.cbWndExtra		= NULL;
	window_class.hInstance		= m_instance_handle;
	window_class.hIcon			= LoadIcon(m_instance_handle, MAKEINTRESOURCE(102)); // TODO: Shouldn't hard-code this, but I don't want to include from the game project -_-.
	window_class.hIconSm		= window_class.hIcon;
	window_class.hCursor		= LoadCursor(NULL, IDC_ARROW);
	window_class.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	window_class.lpszMenuName	= NULL;

#ifdef USE_WINW_UNICODE
	window_class.lpszClassName	= L"Engine_Win32_GfxDisplay";//m_class_name;
#else
	window_class.lpszClassName	= m_class_name;
#endif

	// Register the window class.
	{
#ifdef USE_WINW_UNICODE
		ATOM ret = RegisterClassExW(&window_class);
#else
		ATOM ret = RegisterClassExA(&window_class);
#endif
		DBG_ASSERT(ret != 0);
	}

	switch (m_mode)
	{
	case GfxDisplayMode::Fullscreen:
		{
			window_rect.left	= 0;
			window_rect.top		= 0;
			window_rect.right	= window_rect.left + m_width;
			window_rect.bottom	= window_rect.top + m_height;

			// Change graphics mode to fullscreen.
			DEVMODE dmScreenSettings;                  
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));      

			dmScreenSettings.dmSize				= sizeof(dmScreenSettings);      
			dmScreenSettings.dmPelsWidth		= m_width;            
			dmScreenSettings.dmPelsHeight		= m_height;       
			dmScreenSettings.dmDisplayFrequency	= m_hertz;
			dmScreenSettings.dmBitsPerPel		= 32;            
			dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			if (m_display_changed)
			{
				ChangeDisplaySettings(NULL, 0);
				m_display_changed = false;
			}

			int ret = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
			if (ret == DISP_CHANGE_SUCCESSFUL)
			{
				m_display_changed = true;
				dw_extented_style	= WS_EX_APPWINDOW; 
				dw_style			= WS_POPUP;
				break;
			}
			else
			{
				// Fallthrough to windowed fullscreen if we couldn't change display settings.
			}
		}
	case GfxDisplayMode::FullscreenWindowed:
		{
			if (m_display_changed)
			{
				ChangeDisplaySettings(NULL, 0);
				m_display_changed = false;
			}

			screen_width		= GetSystemMetrics(SM_CXSCREEN);
			screen_height		= GetSystemMetrics(SM_CYSCREEN);

			window_rect.left	= 0;
			window_rect.top		= 0;
			window_rect.right	= window_rect.left + screen_width;
			window_rect.bottom	= window_rect.top + screen_height;

			dw_extented_style	= WS_EX_APPWINDOW; 
			dw_style			= WS_POPUP;

			m_width				= screen_width;
			m_height			= screen_height;

			break;
		}
	case GfxDisplayMode::Windowed:
		{
			if (m_display_changed)
			{
				ChangeDisplaySettings(NULL, 0);
				m_display_changed = false;
			}

			screen_width		= GetSystemMetrics(SM_CXSCREEN);
			screen_height		= GetSystemMetrics(SM_CYSCREEN);

			window_rect.left	= (screen_width / 2) - (m_width / 2);
			window_rect.top		= (screen_height / 2) - (m_height / 2);
			window_rect.right	= window_rect.left + m_width;
			window_rect.bottom	= window_rect.top + m_height;

			break;
		}
	}

	// If sub-client, change position.
	if (*EngineOptions::client_id != 0 || *EngineOptions::spawn_clients != 0)
	{
		int client = *EngineOptions::client_id;

		window_rect.left	= 80 + ((client % 3) * (m_width + 15));
		window_rect.top		= 80 + ((client / 3) * (m_height + 50));
		window_rect.right	= window_rect.left + m_width;
		window_rect.bottom	= window_rect.top + m_height;
	}

	// Make sure window rect is correct.
	{
		BOOL ret = AdjustWindowRectEx(&window_rect, dw_style, false, dw_extented_style);
		DBG_ASSERT(ret != 0);
	}

	// Create window.
	{
		m_window_handle = CreateWindowEx
			(
				dw_extented_style,
				m_class_name,
#ifdef UNICODE 
				wchar_title,
#else
				m_title,
#endif
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dw_style,
				window_rect.left,
				window_rect.top,
				window_rect.right - window_rect.left,
				window_rect.bottom  - window_rect.top,
				NULL,
				NULL,
				m_instance_handle,
				NULL
			);
		DBG_ASSERT(m_window_handle != NULL);
		DBG_ASSERT(IsWindowUnicode(m_window_handle));
	}

	// Attach extra data to window handle.
	{
		LONG_PTR ret = SetWindowLong(m_window_handle, GWLP_USERDATA, reinterpret_cast<LONG>(this));
		DBG_ASSERT(ret == 0);
	}

	// Setup pixel format.
	PIXELFORMATDESCRIPTOR pixel_format;
	memset(&pixel_format, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pixel_format.nSize				= sizeof(PIXELFORMATDESCRIPTOR),                 
	pixel_format.nVersion			= 1;
	pixel_format.dwFlags			= PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixel_format.iPixelType			= PFD_TYPE_RGBA;
	pixel_format.cColorBits			= color_depth;
	pixel_format.cRedBits			= 0;
	pixel_format.cRedShift			= 0;
	pixel_format.cGreenBits			= 0;
	pixel_format.cGreenShift		= 0;
	pixel_format.cBlueBits			= 0;
	pixel_format.cBlueShift			= 0;
	pixel_format.cAlphaBits			= 0;        
	pixel_format.cAlphaShift		= 0;
	pixel_format.cAccumBits			= 0;
	pixel_format.cAccumRedBits		= 0;
	pixel_format.cAccumGreenBits	= 0;
	pixel_format.cAccumBlueBits		= 0;
	pixel_format.cAccumAlphaBits	= 0;
	pixel_format.cDepthBits			= depth_depth;
	pixel_format.cStencilBits		= stencil_depth;
	pixel_format.cAuxBuffers		= 0;
	pixel_format.iLayerType			= PFD_MAIN_PLANE;
	pixel_format.bReserved			= 0;
	pixel_format.dwLayerMask		= 0;
	pixel_format.dwVisibleMask		= 0;
	pixel_format.dwDamageMask		= 0;       

	// Get device context.
	{
		m_device_context = GetDC(m_window_handle);
		DBG_ASSERT(m_device_context != NULL);
	}
	
	// Find and select pixel format.
	{
		GLuint format = ChoosePixelFormat(m_device_context, &pixel_format);
		DBG_ASSERT(format != NULL);

		BOOL ret = SetPixelFormat(m_device_context, format, &pixel_format);
		DBG_ASSERT(ret == TRUE);
	}

	// Create rendering context.
	{
		m_render_context = wglCreateContext(m_device_context);
		DBG_ASSERT(m_render_context != NULL);
	}
	
	// Make rendering context current.
	{
		BOOL result = wglMakeCurrent(m_device_context, m_render_context);
		DBG_ASSERT(result == TRUE);
	}

	// Hide cursor.
	ShowCursor(false);

	// Show window.
	ShowWindow(m_window_handle, SW_SHOW);
	SetForegroundWindow(m_window_handle);
	SetFocus(m_window_handle);

	// Update console.
	Update_Console();

	// IME is not supported - yet - as it has issues with bringing up the IME box in fullscreen/during non-input times.
	// Will will still accept unicode chars through normal WM_CHAR messages.
	ImmDisableIME(-1);
}

void Win32_GfxDisplay::Destroy_Window()
{
	// Destroy rendering context.
	if (m_render_context)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_render_context);
		m_render_context = NULL;
	}

	// Destroy device context.
	if (m_device_context)
	{
		ReleaseDC(m_window_handle, m_device_context);
		m_device_context = NULL;
	}

	// Re-enable mouse cursor.
	ShowCursor(true);

	// Leave fullscreen.
	if (m_display_changed)
	{
		ChangeDisplaySettings(NULL, 0);
		m_display_changed = false;
	}

	// Destroy HWND.
	if (m_window_handle)
	{
		DestroyWindow(m_window_handle);
		m_window_handle = NULL;
	}

	// Unregister instance.
	if (m_instance_handle != NULL)
	{
		UnregisterClass(m_class_name, m_instance_handle);
		m_instance_handle = NULL;
	}
}

void Win32_GfxDisplay::Swap_Buffers()
{
	{
		PROFILE_SCOPE("SwapBuffers");
		SwapBuffers(m_device_context);
	}
}

std::vector<GfxDisplayResolution> Win32_GfxDisplay::Get_Available_Resolutions()
{
	std::vector<GfxDisplayResolution> results;

	for (int i = 0; ; i++)
	{
		DEVMODE mode;
		memset(&mode, 0, sizeof(DEVMODE));
		BOOL result = EnumDisplaySettings(NULL, i, &mode);
		if (result == false)
		{
			break;
		}

		// We don't give a toss about stretch/center display modes.
		if (mode.dmDisplayFixedOutput != DMDFO_DEFAULT)
		{
			continue;
		}

		// We only care about 32bit displays.
		if (mode.dmBitsPerPel != 32)
		{
			continue;
		}

		// We only allow 720p or above.
	//	if (mode.dmPelsHeight < 720)
	//	{
	//		continue;
	//	}

		GfxDisplayResolution res;
		res.Width	= mode.dmPelsWidth;
		res.Height	= mode.dmPelsHeight;
		res.Depth	= mode.dmBitsPerPel;
		res.Hertz	= mode.dmDisplayFrequency;

		results.push_back(res);
	}

	return results;
}

#endif