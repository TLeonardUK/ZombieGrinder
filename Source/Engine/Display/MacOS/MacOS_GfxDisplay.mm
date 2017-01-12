// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/Display/MacOS/MacOS_GfxDisplay.h"
#include "Engine/Input/MacOS/MacOS_MouseState.h"
#include "Engine/Input/MacOS/MacOS_KeyboardState.h"
#include "Engine/Engine/GameEngine.h"

#include <math.h>
#include <string>

#include <AppKit/AppKit.h>
#include <Carbon/Carbon.h>

#if defined( MAC_OS_X_VERSION_10_6 ) && ( MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6 )
@interface OpenGLWindow : NSWindow <NSWindowDelegate>
#else
@interface OpenGLWindow : NSWindow
#endif
{

}
@end

@implementation OpenGLWindow

-(void)sendEvent:(NSEvent*)evt
{
	MacOS_GfxDisplay* display = static_cast<MacOS_GfxDisplay*>(GfxDisplay::Get());
	display->Handle_Event(evt, [self contentView]);

	switch([evt type])
	{
	case NSKeyDown:
	case NSKeyUp:
		// Disable beeps when in fullscreen.
		return;
	}
	[super sendEvent:evt];
}

-(BOOL)windowShouldClose:(id)sender
{
	DBG_LOG("Recieved quit event.");
	GameEngine::Get()->Stop();
	return NO;
}

-(BOOL)canBecomeKeyWindow 
{
    return YES;
}

-(BOOL)canBecomeMainWindow
{
  return YES;
}

@end

static bool Calculate_Mouse_Position(NSView* view, bool display_captured, int* x, int* y)
{
	if (view == NULL || display_captured == true)
	{
		Point pt;
		GetMouse(&pt);

		*x = pt.h;
		*y = pt.v;

		return true;
	}
	else
	{
		NSWindow* window = [view window];
		NSPoint point = [window mouseLocationOutsideOfEventStream];
		NSRect rect = [view bounds];
		point = [view convertPoint:point fromView:nil];

		if (![view isFlipped])
		{
			point.y = rect.size.height - point.y;
		}

		*x = point.x;
		*y = point.y;

		return point.x >= 0 && 
			   point.y >= 0 && 
			   point.x < rect.size.width && 
			   point.y < rect.size.height;
	}
}

MacOS_GfxDisplay::MacOS_GfxDisplay(const char* title, int width, int height, int hertz, GfxDisplayMode::Type mode)
	: m_width(width)
	, m_height(height)
	, m_mode(mode)
	, m_hertz(hertz)
	, m_active(false)
	, m_ns_window(NULL)
	, m_ns_gl_context(NULL)
	, m_resolution_changed(false)
	, m_is_display_captured(false)
	, m_fullscreen_depth(24)
{	
	memset(m_title, 0, MAX_TITLE_LENGTH);
	strcpy(m_title, title);

	Setup_Window();
}  
	
MacOS_GfxDisplay::~MacOS_GfxDisplay()
{
	Destroy_Window();
}   

const char* MacOS_GfxDisplay::Get_Title()
{
	return m_title;
}

int MacOS_GfxDisplay::Get_Width()
{
	return m_window_area_width;
}

int MacOS_GfxDisplay::Get_Height()
{
	return m_window_area_height;
}

int MacOS_GfxDisplay::Get_Hertz()
{
	return m_hertz;
}

GfxDisplayMode::Type MacOS_GfxDisplay::Get_Mode()
{
	return m_mode;
}

bool MacOS_GfxDisplay::Is_Active()
{
	return m_active;
}

void MacOS_GfxDisplay::Set_Title(const char* title)
{
	if (strcmp(m_title, title) == 0)
	{
		return;
	}

	strcpy(m_title, title);
	[(OpenGLWindow*)m_ns_window setTitle:[NSString stringWithUTF8String:m_title]];
}

void MacOS_GfxDisplay::Set_Hidden(bool bHidden)
{
	if (m_ns_window)
	{
		[(OpenGLWindow*)m_ns_window setHidden:YES];
		Reset_Resolution();
	}
}

void MacOS_GfxDisplay::Calculate_Window_Area()
{
	switch (m_mode)
	{
	case GfxDisplayMode::Fullscreen:
		{
		//	m_window_area_x = 0;
		//	m_window_area_y = 0;
		//	m_window_area_width = m_width;
		//	m_window_area_height = m_height;
			m_window_area_x = 0;
			m_window_area_y = 0;
			m_window_area_width = m_original_resolution_width;
			m_window_area_height = m_original_resolution_height;
			break;
		}
	case GfxDisplayMode::FullscreenWindowed:
		{
			m_window_area_x = 0;
			m_window_area_y = 0;
			m_window_area_width = m_original_resolution_width;
			m_window_area_height = m_original_resolution_height;
			break;
		}
	case GfxDisplayMode::Windowed:
		{
			m_window_area_width = m_width;
			m_window_area_height = m_height;
			m_window_area_x = (m_original_resolution_width / 2) - (m_window_area_width / 2);
			m_window_area_y = (m_original_resolution_height / 2) - (m_window_area_height / 2);
			break;
		}
	}
}

bool MacOS_GfxDisplay::Change_Resolution(int width, int height)
{
	if (m_resolution_changed == true)
	{
		Reset_Resolution();
	}

	// Capture control of displays so other apps can't do anything.
	CGCaptureAllDisplays();

	// Get closest matching display resolution.
	CFDictionaryRef displayMode;
	if (m_hertz != 0)
	{
		displayMode = CGDisplayBestModeForParametersAndRefreshRate(kCGDirectMainDisplay, 24, width, height, m_hertz, 0);
	}
	else
	{
		displayMode = CGDisplayBestModeForParameters(kCGDirectMainDisplay, 24, width, height, 0);
	}
	
	// Store the fullscreen state.
	CFNumberRef number = (CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayBitsPerPixel);
	CFNumberGetValue(number, kCFNumberLongType, &m_fullscreen_depth);
	
	number = (CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayWidth);
	CFNumberGetValue(number, kCFNumberLongType, &m_window_area_width);

	number = (CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayHeight);
	CFNumberGetValue(number, kCFNumberLongType, &m_window_area_height);

	DBG_LOG("Changed to resolution: Size=%ix%i Depth=%i", m_window_area_width, m_window_area_height, m_fullscreen_depth);

	// Switch to the new mode!
	int ret = CGDisplaySwitchToMode(kCGDirectMainDisplay, displayMode);
	if (ret != 0)
	{
		DBG_LOG("Failed to change display mode, CGDisplaySwitchToMode returned error 0x%08x.", ret);
		return false;
	}

	// Hide the mouse cursor.
	CGDisplayHideCursor(kCGDirectMainDisplay);

	m_width = m_window_area_width;
	m_height = m_window_area_height;

	m_resolution_changed = true;
	return true;
}

void MacOS_GfxDisplay::Reset_Resolution()
{
	if (m_resolution_changed == false)
	{
		return;
	}

	DBG_LOG("Resetting resolution.");
	
	// Release control of displays so other apps can do things again.
	CGReleaseAllDisplays();

	// Hide the mouse cursor.
	CGDisplayShowCursor(kCGDirectMainDisplay);

	m_resolution_changed = false;
}

void MacOS_GfxDisplay::Setup_Window()
{
	DBG_LOG("Setting up carbon display ...");
	
	CFDictionaryRef displayMode = (CFDictionaryRef)CGDisplayCurrentMode(kCGDirectMainDisplay);
	CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayWidth), kCFNumberLongType, &m_original_resolution_width);
	CFNumberGetValue((CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayHeight), kCFNumberLongType, &m_original_resolution_height);
	
	DBG_LOG("Current Resolution: %i,%i", m_original_resolution_width, m_original_resolution_height);

	Calculate_Window_Area();
	
	// Setup the window.
	switch (m_mode)
	{
		case GfxDisplayMode::Fullscreen:
		/*	{
				// If we can't change resolution, fallback to windowed mode and try again.
				if (!Change_Resolution(m_width, m_height))
				{
					DBG_LOG("Falling back to windowed mode.");
					m_mode = GfxDisplayMode::FullscreenWindowed;
					Setup_Window();
					return;
				}

				m_ns_window = NULL;

				break;
			}*/
		case GfxDisplayMode::FullscreenWindowed:
		case GfxDisplayMode::Windowed:
			{
				int style = NSClosableWindowMask;
				if (m_mode == GfxDisplayMode::Windowed)
				{
					style |= NSTitledWindowMask;
				}
				NSRect rect = NSMakeRect(0, 0, m_window_area_width, m_window_area_height);
		
				OpenGLWindow* window = [[OpenGLWindow alloc]
					initWithContentRect:rect
					styleMask:style
					backing:NSBackingStoreBuffered
					defer:NO];
				DBG_ASSERT_STR(window != NULL, "Failed to create carbon window.");

				[window setDelegate:window];
				[window setAcceptsMouseMovedEvents:YES];
				[window setTitle:[NSString stringWithUTF8String:m_title]];
				[window center];
				[window makeKeyAndOrderFront:NSApp];

				if (m_mode == GfxDisplayMode::Fullscreen)
				{
					[window setLevel: NSMainMenuWindowLevel + 1];
			//		[window setOpaque:YES];
			//		[window setHidesOnDeactivate:YES];
				}
				
				m_ns_window = window;

				break;
			}
	}
	
	DBG_LOG("Creating opengl context.");

	// Work out which attributes our gl context needs to be created with.
	NSOpenGLPixelFormatAttribute attributes[16];
	int attribute_count = 0;
	
	attributes[attribute_count++] = NSOpenGLPFADoubleBuffer;

	//if (m_mode == GfxDisplayMode::Fullscreen)
	//{
	//	attributes[attribute_count++] = NSOpenGLPFAScreenMask;			attributes[attribute_count++] = (NSOpenGLPixelFormatAttribute)CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
	//	attributes[attribute_count++] = NSOpenGLPFAFullScreen;
	//}
	//else
	//{
		attributes[attribute_count++] = NSOpenGLPFANoRecovery;	
	//}

	// Note: This prevents vmware running our shiz ;_;.
	//attributes[attribute_count++] = NSOpenGLPFAAccelerated;

	attributes[attribute_count++] = NSOpenGLPFAColorSize;				attributes[attribute_count++] = (NSOpenGLPixelFormatAttribute)m_fullscreen_depth;
	attributes[attribute_count++] = NSOpenGLPFADepthSize;				attributes[attribute_count++] = (NSOpenGLPixelFormatAttribute)16;
	attributes[attribute_count] = (NSOpenGLPixelFormatAttribute)NULL;

	// Now create the context with said attributes
	NSOpenGLPixelFormat* format  = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];

	// If we can't find a fullscreen format, go back to windowed.
	//if (format == NULL && m_mode == GfxDisplayMode::Fullscreen)
	//{		
	//	DBG_LOG("Falling back to windowed mode.");
	//	Reset_Resolution();
	//	m_mode = GfxDisplayMode::FullscreenWindowed;
	//	Setup_Window();
	//	return;
	//}
	//else
	{
		DBG_ASSERT_STR(format != NULL, "Failed to create opengl format.");
	}

	NSOpenGLContext*	 context = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
	DBG_ASSERT_STR(context != NULL, "Failed to create opengl context.");

	[format release];

	// Attach to actual view.
	if (m_mode == GfxDisplayMode::Fullscreen)
	{
		[[(OpenGLWindow*)m_ns_window contentView] enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];		
		DBG_LOG("Entering fullscreen mode.");
	}
	[context setView:[(OpenGLWindow*)m_ns_window contentView]];

	// Disable vsync.
	static GLint sync = 0;
	[context setValues:(GLint*)&sync forParameter:(NSOpenGLContextParameter)kCGLCPSwapInterval];
	
	// And finally update and store the context.
	[context update];
	[context makeCurrentContext];
	m_ns_gl_context = context;
}

void MacOS_GfxDisplay::Destroy_Window()
{
	//if (m_mode == GfxDisplayMode::Fullscreen)
	//{
	//	Reset_Resolution();
	//}

	[NSOpenGLContext clearCurrentContext];
	[(NSOpenGLContext*)m_ns_gl_context clearDrawable];
	[(NSOpenGLContext*)m_ns_gl_context release];
	[(OpenGLWindow*)m_ns_window close];
}

void MacOS_GfxDisplay::Set_VSync(bool enabled)
{
	// Disable vsync.
	static GLint sync = enabled ? 1 : 0;
	[(NSOpenGLContext*)m_ns_gl_context setValues:(GLint*)&sync forParameter:(NSOpenGLContextParameter)kCGLCPSwapInterval];
}

bool MacOS_GfxDisplay::Resize(int width, int height, int hertz, GfxDisplayMode::Type mode)
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
	
	// Reset to normal graphics mode.	
	//if (old_mode == GfxDisplayMode::Fullscreen && m_mode != GfxDisplayMode::Fullscreen)
	//{
	//	Reset_Resolution();		
	//}

	// Calculate new window area().
	Calculate_Window_Area();

	// Get current window.
	OpenGLWindow* window = (OpenGLWindow*)m_ns_window;
	NSOpenGLContext* context = (NSOpenGLContext*)m_ns_gl_context;

	// Exit fullscreen.
	if (m_mode != GfxDisplayMode::Fullscreen &&
		m_old_mode == GfxDisplayMode::Fullscreen)
	{
		[[window contentView] exitFullScreenModeWithOptions:nil];
		DBG_LOG("Exiting fullscreen mode.");
	}

	// Resize the window.
	int style = NSClosableWindowMask;
	if (m_mode == GfxDisplayMode::Windowed)
	{
		style |= NSTitledWindowMask;
	}

	NSRect rect = NSMakeRect(m_window_area_x, m_window_area_y, m_window_area_width, m_window_area_height);
	[window setStyleMask:style];
	[window setFrame:rect display:YES];
	
	DBG_LOG("Changing display to: Mode=%i Area=%i,%i,%i,%i", m_mode, m_window_area_x, m_window_area_y, m_window_area_width, m_window_area_height);

	if (m_mode == GfxDisplayMode::Fullscreen)
	{
		[[(OpenGLWindow*)m_ns_window contentView] enterFullScreenMode:[NSScreen mainScreen] withOptions:nil];
		DBG_LOG("Entering fullscreen mode.");
	}
	
	if (m_mode == GfxDisplayMode::Fullscreen)
	{
		[window setLevel: NSMainMenuWindowLevel + 1];
	}
	else
	{
		[window setLevel: NSNormalWindowLevel];
	}
		
	[window makeKeyAndOrderFront:NSApp];
	[window center];
	
	[context flushBuffer];
	[context update];

	return true;
}

void MacOS_GfxDisplay::Tick(const FrameTime& time)
{
	m_is_display_captured = CGDisplayIsCaptured(kCGDirectMainDisplay);
	
	// Update active flag.
	if ((m_ns_window == NULL && m_is_display_captured) || m_mode == GfxDisplayMode::Fullscreen)
	{
		m_active = true;
	}
	else if (m_ns_window != NULL)
	{
		// Note: This seems wrong? Window because always active as soon as you click on it, wut?
		m_active = [(OpenGLWindow*)m_ns_window isKeyWindow];
	}
	else
	{
		m_active = false;
	}

	// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
	// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
	//static int ticks = 0;
	//if (ticks++ > 1300)
	//{
	//	exit(0);
	//}
	// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG
	// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG

	// Update mouse state with new position.
	int x = 0, y = 0;
	Calculate_Mouse_Position(m_ns_window != NULL && m_mode != GfxDisplayMode::Fullscreen ? [(OpenGLWindow*)m_ns_window contentView] : NULL, m_is_display_captured, &x, &y);
	MacOS_MouseState::Post_Mouse_Position(x, y);

	// Chug through events.
	while (true)
	{
		NSEvent* evt = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
		if (evt == NULL)
		{
			break;
		}
		else if ([evt type] == NSApplicationDefined)
		{
			// We don't use any of these yet.
		}
		else
		{
			if (m_is_display_captured == true)
			{
				Handle_Event(evt, NULL);
			}
			else
			{
				[NSApp sendEvent:evt];
			}
		}
	}
}

void MacOS_GfxDisplay::Handle_Event(void* evt_pimpl, void* view_pimpl)
{
	NSEvent* evt = (NSEvent*)evt_pimpl;
	NSView* view = (NSView*)view_pimpl;

	NSEventType type = [evt type];

	switch (type)
	{
		case NSKeyDown:
			{
				// Send character input.
				NSString* chars = [evt characters];
				if ([chars length] == 1)
				{					
					// TODO: UNICODE

					int chr = [chars characterAtIndex:0];
					if (chr == 127) 
					{
						chr = 8;
					}
					if (chr == 63272) 
					{
						chr = 127;
					}
					if (chr <= 0xf700 || chr >= 0xf8ff)
					{
						MacOS_KeyboardState::Post_Character(chr);
					}
				}
				break;
			}
		case NSKeyUp:
			{
				break;
			}
		case NSFlagsChanged:
			{
				break;
			}
		case NSLeftMouseDown:
		case NSRightMouseDown:
		case NSOtherMouseDown:
			{
				int button = (type == NSLeftMouseDown) ? 0 : (type == NSRightMouseDown ? 1 : 2);				
				MacOS_MouseState::Post_Mouse_Down(button);
				break;
			}
		case NSLeftMouseUp:
		case NSRightMouseUp:
		case NSOtherMouseUp:
			{
				int button = (type == NSLeftMouseUp) ? 0 : (type == NSRightMouseUp ? 1 : 2);
				MacOS_MouseState::Post_Mouse_Up(button);	
				break;
			}
		case NSMouseMoved:
		case NSLeftMouseDragged:
		case NSRightMouseDragged:
		case NSOtherMouseDragged:
			{
				break;
			}
		case NSScrollWheel:
			{
				float delta = [evt deltaY];
				delta = delta > 0 ? ceilf(delta) : floorf(delta);
				MacOS_MouseState::Post_Wheel_Event(delta);
				break;
			}
		default:
			{
				break;
			}
	}
}

void MacOS_GfxDisplay::Swap_Buffers()
{
	[(NSOpenGLContext*)m_ns_gl_context flushBuffer];
}

std::vector<GfxDisplayResolution> MacOS_GfxDisplay::Get_Available_Resolutions()
{
	std::vector<GfxDisplayResolution> results;

	CFArrayRef display_modes = CGDisplayAvailableModes(kCGDirectMainDisplay);
	int display_mode_count = CFArrayGetCount(display_modes);
	
	for (int i = 0; i < display_mode_count; i++)
	{
		GfxDisplayResolution resolution;

		CFDictionaryRef displayMode = (CFDictionaryRef)CFArrayGetValueAtIndex(display_modes, i);

		CFNumberRef number = (CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayBitsPerPixel);
		CFNumberGetValue(number, kCFNumberLongType, &resolution.Depth);

		number = (CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayWidth);
		CFNumberGetValue(number, kCFNumberLongType, &resolution.Width);

		number = (CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayHeight);
		CFNumberGetValue(number, kCFNumberLongType, &resolution.Height);

		number = (CFNumberRef)CFDictionaryGetValue(displayMode, kCGDisplayRefreshRate);
		CFNumberGetValue(number, kCFNumberLongType, &resolution.Hertz);

		if (resolution.Depth > 16)
		{
			results.push_back(resolution);
		}
	}
	
	return results;
}
 
#endif