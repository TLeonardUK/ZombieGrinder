// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#import <AppKit/AppKit.h>

int macos_main(int argc, char* argv[]);
void macos_end();

NSMutableArray*		g_app_args;
NSAutoreleasePool*	g_global_pool;
int					g_argc;
char**				g_argv;

@interface EngineApp : NSApplication
{
}
@end

@implementation EngineApp
@end

@interface EngineAppDelegate : NSObject
{
}
@end

@implementation EngineAppDelegate
	-(void)applicationWillTerminate:(NSNotification*)notification
	{
		DBG_LOG("EngineAppDelegate: applicationWillTerminate invoked.");
		macos_end();
	}

	-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
	{
		return NSTerminateCancel;
	}
	
	- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication 
	{
	    return YES;
	}

	-(BOOL)application:(NSApplication*)app openFile:(NSString*)path
	{
		// TODO: Handle file opens.
		DBG_LOG("EngineAppDelegate: openFile invoked.");
		return YES;
	}

	-(void)applicationDidFinishLaunching:(NSNotification*)notification
	{
		DBG_LOG("EngineAppDelegate: applicationDidFinishLaunching invoked.");
		[NSApp activateIgnoringOtherApps:YES];
		macos_main(g_argc, g_argv);
	}
@end

void Create_App_Menu(NSString *appName)
{
	[NSApp setMainMenu:[NSMenu new]];
	
	NSMenu* appMenu = [NSMenu new];
	
	NSString* title = [@"Hide " stringByAppendingString:appName];
	[appMenu addItemWithTitle:@"Hide" action:@selector(hide:) keyEquivalent:@"h"];

	NSMenuItem * item = (NSMenuItem*)[appMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[item setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];
	
	[appMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
	
	[appMenu addItem:[NSMenuItem separatorItem]];

	title = [@"Quit " stringByAppendingString:appName];
	[appMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];
	
	item  =[NSMenuItem new];
	[item setSubmenu:appMenu];
	[[NSApp mainMenu] addItem:item];
	
	[NSApp performSelector:NSSelectorFromString(@"setAppleMenu:") withObject:appMenu];
}

void Show_Alert(const char* inTitle, const char* inMessage)
{
	NSString* title = [NSString stringWithUTF8String:inTitle];
	NSString* message = [NSString stringWithUTF8String:inMessage];

	NSRunCriticalAlertPanel(title, message, @"OK", NULL, NULL);
}

int main(int argc, char* argv[])
{	
	g_global_pool = [[NSAutoreleasePool alloc] init];

	g_argc = argc;
	g_argv = argv;

	[EngineApp sharedApplication];

	Create_App_Menu([NSString stringWithCString:PROJECT_TITLE]);
	
	[NSApp setDelegate:[[EngineAppDelegate alloc] init]];
		
	g_app_args = [[NSMutableArray arrayWithCapacity:10] retain];
	[g_app_args addObject:[NSString stringWithCString:argv[0]] ];
			
	[NSApp run];
}

#endif