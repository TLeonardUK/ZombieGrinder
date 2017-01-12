// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Engine/Engine/GameEngine.h"
#include "Engine/Display/Linux/Linux_GfxDisplay.h"

#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "Engine/Input/Linux/Linux_MouseState.h"
#include "Engine/Input/Linux/Linux_KeyboardState.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include "Generic/Helper/StringHelper.h"

#include <stdio.h>
#include <string>

extern char* g_executable_file_path;

// This whole file is a total clusterfuck of crazy. X11 is the devils spawn.
// Some real wizardry going on ...

// TODO:
//	Fullscreen -> windowed -> Windowed : We loose border, why?

// Why the fuck are these not defined anywhere?
#define V_INTERLACE	0x010 
#define V_DBLSCAN	0x020

#define _NET_WM_STATE_REMOVE    0l
#define _NET_WM_STATE_ADD       1l
#define _NET_WM_STATE_TOGGLE    2l

static char g_blank_cursor_data[1] = {0};

// Our output format. RGBA
static int g_render_format[] =                                             
{                                                                      
    GLX_RGBA,			GLX_DOUBLEBUFFER,                                        
    GLX_RED_SIZE,		8,                                                   
    GLX_GREEN_SIZE,		8,                                                 
    GLX_BLUE_SIZE,		8,                                                  
    GLX_DEPTH_SIZE,		16,                                                
    None                                                               
};    

static Bool WaitForMap(Display *display, XEvent *event, XPointer arg)
{
	return (event->type == MapNotify) && (event->xmap.window == (Window)arg);
}

static Bool WaitForUnmap(Display *display, XEvent *event, XPointer arg)
{
	return (event->type == UnmapNotify) && (event->xmap.window == (Window)arg);
}

static Bool WaitForConfigure(Display *display, XEvent *event, XPointer arg)
{
	return (event->type == ConfigureNotify) && (event->xmap.window == (Window)arg);
}

Linux_GfxDisplay::Linux_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode)
	: m_width(width)
	, m_height(height)
	, m_mode(mode)
	, m_hertz(hertz)
	, m_active(false)
	, m_icon_data(NULL)
	, m_inputMethod(0)
	, m_inputContext(0)
{	
	memset(m_title, 0, MAX_TITLE_LENGTH);
	strcpy(m_title, title);

	m_display = XOpenDisplay(NULL);
	DBG_ASSERT(m_display != NULL);

	m_root_window = XDefaultRootWindow(m_display);

	// Make blank cursor. (Why is there no hide-cursor option in x11 >_>)
	XColor dummy_color;
	Pixmap pixmap = XCreateBitmapFromData(m_display, m_root_window, g_blank_cursor_data, 1, 1);
	m_blank_cursor = XCreatePixmapCursor(m_display, pixmap, pixmap, &dummy_color, &dummy_color, 0, 0);
	XFreePixmap(m_display, pixmap);
 
	// Load the Pixelmap icon.
	std::string icon_path = StringHelper::Format("%s.png", g_executable_file_path);
	Pixelmap* map = PixelmapFactory::Load(icon_path.c_str());
	if (map != NULL)
	{
		m_icon_data_len = 2 + (map->Get_Width() * map->Get_Height());
		m_icon_data = new int[m_icon_data_len];

		m_icon_data[0] = map->Get_Width();
		m_icon_data[1] = map->Get_Height();

		int* data_ptr = m_icon_data + 2;
		for (int y = 0; y < map->Get_Height(); y++)
		{
			for (int x = 0; x < map->Get_Width(); x++)
			{
				*data_ptr = map->Get_Pixel(x, y).To_ARGB();
				data_ptr++;
			}
		}

		SAFE_DELETE(map);

		DBG_LOG("Loaded game icon from: %s (length %i)", icon_path.c_str(), m_icon_data_len);
	}
	else
	{
		DBG_LOG("Failed to load game icon from: %s", icon_path.c_str());
	}

	Setup_Window();
}
	
Linux_GfxDisplay::~Linux_GfxDisplay()
{
	Destroy_Window();

	if (m_icon_data != NULL)
		delete[] m_icon_data;

	if (m_blank_cursor != NULL)
		XFreeCursor(m_display, m_blank_cursor);
}

const char* Linux_GfxDisplay::Get_Title()
{
	return m_title;
}

int Linux_GfxDisplay::Get_Width()
{
	return m_width;
}

int Linux_GfxDisplay::Get_Height()
{
	return m_height;
}

int Linux_GfxDisplay::Get_Hertz()
{
	return m_hertz;
}

GfxDisplayMode::Type Linux_GfxDisplay::Get_Mode()
{
	return m_mode;
}

bool Linux_GfxDisplay::Is_Active()
{
	return m_active;
}

void Linux_GfxDisplay::Set_Title(const char* title)
{
	if (strcmp(m_title, title) == 0)
	{
		return;
	}

	strcpy(m_title, title);
	XStoreName(m_display, m_window, title);
}

void Linux_GfxDisplay::Set_Hidden(bool bHidden)
{
	if (m_display && m_window)
	{
		XUnmapWindow(m_display, m_window);

		if (m_mode == GfxDisplayMode::Fullscreen)
		{
			XF86VidModeLockModeSwitch(m_display, m_default_screen, False);

			Change_Resolution(&m_desktop_mode, true);

			XUngrabKeyboard(m_display, CurrentTime);
			XUngrabPointer(m_display, CurrentTime);
		}
	}
}

bool Linux_GfxDisplay::Resize(int width, int height, int hertz, GfxDisplayMode::Type mode)
{	
	if (m_width		== width &&
		m_height	== height &&
		m_hertz		== hertz &&
		m_mode		== mode)
	{
		return false;
	}
	
	GfxDisplayMode::Type old_mode = m_mode;

	m_width = width;
	m_height = height;
	m_hertz = hertz;
	m_mode = mode;

	DBG_LOG("Resizing graphics display ...");
	
	int video_mode_count;
	XF86VidModeModeInfo** video_modes;  
	
	XF86VidModeGetAllModeLines(m_display, m_default_screen, &video_mode_count, &video_modes);  

	int matching_mode = 0; // Use native-res if all else fails.
	for (int i = 0; i < video_mode_count; i++)
	{
		XF86VidModeModeInfo* mode = video_modes[i];
		if (mode->hdisplay == m_width && mode->vdisplay == m_height)
		{
			matching_mode = i;
		}
	}
	
	int window_w = video_modes[matching_mode]->hdisplay;
	int window_h = video_modes[matching_mode]->vdisplay;
	
	// Reset to normal graphics mode.	
	if (old_mode == GfxDisplayMode::Fullscreen && m_mode != GfxDisplayMode::Fullscreen)
	{
		XF86VidModeLockModeSwitch(m_display, m_default_screen, False);

		DBG_LOG("Reverting to desktop resolution.");

		Change_Resolution(&m_desktop_mode, true);

		XUngrabKeyboard(m_display, CurrentTime);                                     
		XUngrabPointer(m_display, CurrentTime);				
	}

	switch (m_mode)
	{
	case GfxDisplayMode::Fullscreen:
		{
			Change_Resolution(video_modes[matching_mode]);
			m_window_attr.override_redirect = False;
			break;
		}
	case GfxDisplayMode::FullscreenWindowed:
		{
			window_w = m_display_width;
			window_h = m_display_height;
			m_window_attr.override_redirect = False;
			break;
		}
	case GfxDisplayMode::Windowed:
		{
			m_window_attr.override_redirect = False;						
			break;
		}
	}
	
	DBG_LOG("Desktop size: %i,%i", m_display_width, m_display_height);
	DBG_LOG("Window size: %i,%i (requested %i,%i in mode %i)", window_w, window_h, m_width, m_height, m_mode);

	// Force apply the changed properties.	
	int attribute_mask = CWOverrideRedirect;
	XChangeWindowAttributes(m_display, m_window, attribute_mask, &m_window_attr);

	// General window properties.
	Set_Window_Properties(m_title, window_w, window_h);		
	Set_Window_Bordered(m_mode == GfxDisplayMode::Windowed);									
	Set_Window_Input(m_mode == GfxDisplayMode::Fullscreen);

	// Position in center of the screen!
	if (m_mode == GfxDisplayMode::Windowed || m_mode == GfxDisplayMode::FullscreenWindowed)
	{
		Set_Window_Location((m_display_width - window_w) / 2, (m_display_height - window_h) / 2, window_w, window_h);
	}
	else
	{
		Set_Window_Location(0, 0, window_w, window_h);
	}

	// Force input on to us (we seem to loose it when changing from fullscreen).
	XSetInputFocus(m_display, m_window, RevertToParent, CurrentTime);
	XClearWindow(m_display, m_window);

	// Apply all changes.
	XFlush(m_display);
	XSync(m_display, False);

	return true;
}

void Linux_GfxDisplay::Setup_Window()
{
	int vidmode_major, vidmode_minor;
	int glx_minor, glx_major;
	int video_mode_count;
	XF86VidModeModeInfo** video_modes;  
	XVisualInfo* visual_info;

    m_default_screen = DefaultScreen(m_display);

    XF86VidModeQueryVersion(m_display, &vidmode_major, &vidmode_minor);
    DBG_LOG("XF86 VideoMode extension version %d.%d", vidmode_major, vidmode_minor);
	                                    
    glXQueryVersion(m_display, &glx_major, &glx_minor);                                     
    DBG_LOG("GLX-Version %d.%d", glx_major, glx_minor); 
	
	XF86VidModeGetModeLine(m_display, m_default_screen, reinterpret_cast<int*>(&m_desktop_mode.dotclock), (XF86VidModeModeLine*)&m_desktop_mode.hdisplay);	
	XF86VidModeGetAllModeLines(m_display, m_default_screen, &video_mode_count, &video_modes);  

	// Find the mode we requested.
	DBG_LOG("Available Video Modes:");

	int matching_mode = 0; // Use native-res if all else fails.
	for (int i = 0; i < video_mode_count; i++)
	{
		XF86VidModeModeInfo* mode = video_modes[i];
		DBG_LOG("\t[%i] %i x %i", i, mode->hdisplay, mode->vdisplay);

		if (mode->hdisplay == m_width && mode->vdisplay == m_height)
		{
			matching_mode = i;
		}
	}

	// Grab a visual for R8G8B8 double buffering.        
    visual_info = glXChooseVisual(m_display, m_default_screen, g_render_format);    
	DBG_ASSERT_STR(visual_info != NULL, "Visual rendering style not available.");

	// Create colormap.
	m_color_map = XCreateColormap(m_display, m_root_window, visual_info->visual, AllocNone);
	m_window_attr.colormap			= m_color_map;
	m_window_attr.border_pixel		= 0;
	m_window_attr.event_mask		= ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask | FocusChangeMask;   
	m_window_attr.cursor			= None;
	m_window_attr.background_pixel  = BlackPixel(m_display, m_default_screen);

	// Create gflx content.
	m_context = glXCreateContext(m_display, visual_info, 0, GL_TRUE);  
	
	m_display_width = m_desktop_mode.hdisplay;
	m_display_height = m_desktop_mode.vdisplay;
	
	int window_w = video_modes[matching_mode]->hdisplay;
	int window_h = video_modes[matching_mode]->vdisplay;
	
	// Create the window.
	switch (m_mode)
	{
	case GfxDisplayMode::Fullscreen:
		{
			Change_Resolution(video_modes[matching_mode]);
			m_window_attr.override_redirect = False;
			break;
		}
	case GfxDisplayMode::FullscreenWindowed:
		{
			window_w = m_display_width;
			window_h = m_display_height;
			m_window_attr.override_redirect = False;
			break;
		}
	case GfxDisplayMode::Windowed:
		{
			m_window_attr.override_redirect = False;
			break;
		}
	}

	DBG_LOG("Desktop size: %i,%i", m_display_width, m_display_height);
	DBG_LOG("Window size: %i,%i (requested %i,%i in mode %i)", window_w, window_h, m_width, m_height, m_mode);

	// Create the window.
	m_window = XCreateWindow(m_display, m_root_window,    
				0, 0, window_w, window_h, 0, visual_info->depth, InputOutput, visual_info->visual,
				CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect | CWBackPixel,   
				&m_window_attr);  

	// No cursor plx.
	XDefineCursor(m_display, m_window, m_blank_cursor);

	// General window properties.
	Set_Window_Icon(m_icon_data, m_icon_data_len);
	Set_Window_Properties(m_title, window_w, window_h);
	Set_Window_Input(m_mode == GfxDisplayMode::Fullscreen);
	Set_Window_Bordered(m_mode == GfxDisplayMode::Windowed);

	// Position in center of the screen!
	if (m_mode == GfxDisplayMode::Windowed || m_mode == GfxDisplayMode::FullscreenWindowed)
	{
		Set_Window_Location((m_display_width - window_w) / 2, (m_display_height - window_h) / 2, window_w, window_h);
	}
	else
	{
		Set_Window_Location(0, 0, window_w, window_h);

		XF86VidModeLockModeSwitch(m_display, m_default_screen, True);
	}																								
	
	// Free our unused structures.
	XFree(video_modes); 

	// Apply all changes.
	XFlush(m_display);
	XSync(m_display, False);
	
	// Connect glx-content to the window.
	glXMakeCurrent(m_display, m_window, m_context);
	if (glXIsDirect(m_display, m_context))
	{
		DBG_LOG("GLX is direct rendering context.");
	}
	else
	{
		DBG_LOG("GLX is not-direct rendering context.");
	}

	// Apply all changes.
	XFlush(m_display);
	XSync(m_display, False);

	// Force input on to us (we seem to loose it when changing from fullscreen).
	//XSetInputFocus(m_display, m_window, RevertToParent, CurrentTime);
	XClearWindow(m_display, m_window);

	// Setup input.
	m_inputMethod = XOpenIM(m_display, NULL, NULL, NULL);
	if (m_inputMethod == NULL)
	{
		DBG_LOG("XOpenIM returned invalid input method!");
	}
	else
	{
		m_inputContext = XCreateIC(m_inputMethod, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, m_window, NULL);
		if (m_inputContext == NULL)
		{
			DBG_LOG("XCreateIC returned invalid result.");
		}
		else
		{
			XSetICFocus(m_inputContext);
		}
	}
}

void Linux_GfxDisplay::Destroy_Window()
{
	if (m_inputContext)
	{
		XDestroyIC(m_inputContext);
		m_inputContext = NULL;
	}

	if (m_inputMethod)
	{
		XCloseIM(m_inputMethod);
		m_inputMethod = NULL;
	}

	if (m_context)
	{
		if (glXMakeCurrent(m_display, None, NULL))
		{
			DBG_LOG("Failed to release drawing context with glXMakeCurrent.");
		}
		glXDestroyContext(m_display, m_context);
		m_context = NULL;
	}

	if (m_mode == GfxDisplayMode::Fullscreen)
	{
		Change_Resolution(&m_desktop_mode, true);
	}

	XCloseDisplay(m_display);
}

void Linux_GfxDisplay::Set_Window_Bordered(bool bordered)
{   
	XEvent event;

	// Based on libsdl code:
	// http://www.libsdl.org/tmp/SDL/src/video/x11/SDL_x11window.c

	Atom WM_HINTS = XInternAtom(m_display, "_MOTIF_WM_HINTS", True);
	if (WM_HINTS != None) 
	{
		/* Hints used by Motif compliant window managers */
		struct
		{
			unsigned long flags;
			unsigned long functions;
			unsigned long decorations;
			long input_mode;
			unsigned long status;
		} MWMHints = 
		{
			(1L << 1), 
			0, 
			bordered == true ? 1 : 0, 
			0, 
			0
		};

		XChangeProperty(m_display, m_window, WM_HINTS, WM_HINTS, 32, PropModeReplace, (unsigned char *)&MWMHints, sizeof(MWMHints) / 4);

		DBG_LOG("Using _MOTIF_WM_HINTS to change window border.");
	} 
	else 
	{  
		XSetTransientForHint(m_display, m_window, m_root_window);

		DBG_LOG("Using XSetTransientForHint to change window border.");
	}

	if (bordered == true)
	{
		DBG_LOG("Adding window border.");
	}
	else
	{
		DBG_LOG("Removing window border.");
	}

	XFlush(m_display);
	XSync(m_display, False);
}

void Linux_GfxDisplay::Set_Window_Icon(int* data, int len)
{ 
	Atom net_wm_icon = XInternAtom(m_display, "_NET_WM_ICON", False);
	Atom cardinal = XInternAtom(m_display, "CARDINAL", False);

	XChangeProperty(m_display, m_window, net_wm_icon, cardinal, 32, PropModeReplace, (const unsigned char*)data, len);
}

void Linux_GfxDisplay::Set_Window_Location(int x, int y, int w, int h)
{
	DBG_LOG("Setting window geometry to %i,%i,%i,%i.", x, y, w, h);

	XResizeWindow(m_display, m_window, w, h);
	XMoveWindow(m_display, m_window, x, y);
	XRaiseWindow(m_display, m_window);

	XFlush(m_display);
	XSync(m_display, False);

	m_width = w;
	m_height = h;
}   

void Linux_GfxDisplay::Set_Window_Input(bool exclusive)
{
	if (exclusive == true)
	{
		XWarpPointer(m_display, None, m_window, 0, 0, 0, 0, 0, 0);  
		XGrabKeyboard(m_display, m_window, True, GrabModeAsync, GrabModeAsync, CurrentTime);                                     
		XGrabPointer(m_display, m_window, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, m_window, None, CurrentTime); 
	}

	//XSelectInput(m_display, m_window, ResizeRedirectMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask);		

	XFlush(m_display);
	XSync(m_display, False);
}

void Linux_GfxDisplay::Change_Resolution(XF86VidModeModeInfo* mode, bool desktop_size)
{
	DBG_LOG("Changing resolution to %i,%i.", mode->hdisplay, mode->vdisplay);

	int video_mode_count;
	XF86VidModeModeInfo** video_modes;  
	XF86VidModeGetAllModeLines(m_display, m_default_screen, &video_mode_count, &video_modes);  

	// Find closest mode.
	int matching_mode = 0; 
	int matching_diff = -1;
	for (int i = 0; i < video_mode_count; i++)
	{
		XF86VidModeModeInfo* m = video_modes[i];
		int diff = abs(m->hdisplay - mode->hdisplay) + abs(m->vdisplay - mode->vdisplay);
		
		if (diff < matching_diff || matching_diff == -1)
		{
			matching_mode = i;
			matching_diff = diff;
		}
	}

	if (desktop_size == true)
	{
		m_display_width  = video_modes[matching_mode]->hdisplay;
		m_display_height = video_modes[matching_mode]->vdisplay;
	}

	// Change mode.
	XF86VidModeSwitchToMode(m_display, m_default_screen, video_modes[matching_mode]);
	XF86VidModeSetViewPort(m_display, m_default_screen, 0, 0); 
	XFlush(m_display);
	XSync(m_display, False);

	// Free our unused structures.
	XFree(video_modes); 
}

void Linux_GfxDisplay::Set_Window_Properties(const char* title, int min_w, int min_h)
{
	Atom delete_atom = XInternAtom(m_display, "WM_DELETE_WINDOW", True);                 
	XSetWMProtocols(m_display, m_window, &delete_atom, 1);          
			
	XSizeHints* hints = XAllocSizeHints();
	hints->min_width = min_w;
	hints->max_width = min_w;
	hints->min_height = min_h;
	hints->max_height = min_h;

	if (m_mode == GfxDisplayMode::Fullscreen)
	{
		hints->flags = 0;
	}
	else
	{
		hints->flags = PMinSize | PMaxSize;
	}

	// Add state change flag.
//	Atom name  = XInternAtom(m_display, "_NET_WM_STATE", True);
//	Atom value = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", True);
//	XChangeProperty(m_display, m_window, name, XA_ATOM, 32, PropModeReplace, (unsigned char*)&value, 1);

	// Apply hints.
	XSetStandardProperties(m_display, m_window, title, title, None, NULL, 0, hints); 

	// Make window visible.
	XMapRaised(m_display, m_window);                                 
	
	XFlush(m_display);
	XSync(m_display, False);

	// Send fullscreen event.
	XEvent evt;
	memset(&evt, 0, sizeof(evt));
	evt.type                 = ClientMessage;
	evt.xclient.message_type = XInternAtom(m_display, "_NET_WM_STATE", False);
	evt.xclient.format       = 32;
	evt.xclient.window       = m_window;
	evt.xclient.data.l[0]    = (m_mode == GfxDisplayMode::Fullscreen) == true ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
	evt.xclient.data.l[1]    = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);
	evt.xclient.data.l[3]	 = 0l;

	XSendEvent(m_display, DefaultRootWindow(m_display), False, SubstructureNotifyMask | StructureNotifyMask, &evt);

	XFlush(m_display);
	XSync(m_display, False);

	XFree(hints);
}

void Linux_GfxDisplay::Tick(const FrameTime& time)
{
	XEvent evt;

	while (XPending(m_display) > 0)        
	{                                    
		XNextEvent(m_display, &evt); 

		switch (evt.type)
		{
		case FocusIn:
			{
				DBG_LOG("Window gained focus.");
				m_active = true;
				break;
			}
		case FocusOut:
			{
				DBG_LOG("Window lost focus.");
				m_active = false;
				break;
			}
		case Expose:
			{
				break;
			}
		case ConfigureNotify:
			{
				break;
			}
		case ButtonPress:
			{
				switch (evt.xbutton.button)
				{
				// Linux_MouseState takes care of other events.

				case Button4:
					{
						Linux_MouseState::Post_Wheel_Event(-1);
						break;
					}
				case Button5:
					{
						Linux_MouseState::Post_Wheel_Event(-1);
						break;
					}
				}
				break;
			}
		case KeyPress:
			{
				char	key_text[20];
				KeySym	key_sym;
				Status  status = 0;
  
  				if (m_inputContext)
  				{  					
					int count = Xutf8LookupString(m_inputContext, (XKeyPressedEvent*)&evt, key_text, 20, &key_sym, &status);
					if (status != XBufferOverflow && count > 0)
					{
						key_text[count] = '\0';
						Linux_KeyboardState::Post_Character(key_text);
					}
				}
				else
				{
					int count = XLookupString(&evt.xkey, key_text, 20, &key_sym, 0);
					if (count > 0)
					{
						key_text[count] = '\0';
						Linux_KeyboardState::Post_Character(key_text);
					}
				}
				
				break;
			}
		case ClientMessage:
			{
				Atom delete_atom = XInternAtom(m_display, "WM_DELETE_WINDOW", True);  
				
	            if ((Atom)evt.xclient.data.l[0] == delete_atom) 
				{
					DBG_LOG("Recieved quit event.");
					GameEngine::Get()->Stop();
				}

				break;
			}
		default:
			{
				break;
			}
		}
	}
}

void Linux_GfxDisplay::Swap_Buffers()
{
	glXSwapBuffers(m_display, m_window);
}

std::vector<GfxDisplayResolution> Linux_GfxDisplay::Get_Available_Resolutions()
{
	int video_mode_count;
	XF86VidModeModeInfo** video_modes;  
	
	XF86VidModeGetAllModeLines(m_display, m_default_screen, &video_mode_count, &video_modes);  

	std::vector<GfxDisplayResolution> results;

	for (int i = 0; i < video_mode_count; i++)
	{
		XF86VidModeModeInfo* mode = video_modes[i];
	
		int hertz = (int)((mode->dotclock * 1000.0) / (mode->htotal * mode->vtotal) + 0.5f);
		if (mode->flags & V_INTERLACE) hertz <<= 1;
		if (mode->flags & V_DBLSCAN)   hertz >>= 1;

		GfxDisplayResolution res;
		res.Width	= mode->hdisplay;
		res.Height	= mode->vdisplay;
		res.Depth	= 24;
		res.Hertz	= hertz;

		results.push_back(res);
	}

	return results;
}

#endif