/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Cocoa/Cocoa.h>
#import <AppKit/NSApplication.h>

#include <hltypes/hlog.h>

#include "Application.h"
#include "april.h"
#include "RenderSystem.h"
#include "Window.h"
#include "main_base.h"
#ifdef _COCOA_WINDOW
#import "Mac_Window.h"
#endif
#import "Mac_AppDelegate.h"

#define MAC_WINDOW ((april::Mac_Window*)april::window)

static BOOL gFinderLaunch = NO;

int gAprilShouldInvokeQuitCallback = 0;

static NSString* getLocalizedString(NSString* key, NSString* fallback)
{
	NSString* s = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:key];
	return (s == nil || [s length] == 0 ? fallback : s);
}

// For some reason, Apple removed setAppleMenu from the headers in 10.4, but the method still is there and works.
// To avoid warnings, we declare it ourselves here.
@interface NSApplication(April_Missing_Methods)
- (void)setAppleMenu:(NSMenu*) menu;
@end

@interface AprilApplication : NSApplication
@property bool appRunning;
@end

@implementation AprilApplication

- (void)terminate:(id)sender
{
	self.appRunning = false;
#ifdef _SDL
	gAprilShouldInvokeQuitCallback = 1;
#else
	if (april::window == NULL)
	{
		[super terminate:sender];
	}
	bool result = true;
	if (april::isUsingCVDisplayLink())
	{
		hmutex::ScopeLock lock(&MAC_WINDOW->renderThreadSyncMutex);
		result = april::window->handleQuitRequest(false);
		lock.release();
	}
	else
	{
		// kspes@20190103 - setting canQuit to false since this code is running on the main thread
		// when Cmd+Q is pressed, so just fuck it, have the game quit without a prompt, don't care, doesn't matter.
		result = april::window->handleQuitRequest(false);
	}
	if (result)
	{
		[super terminate:sender];
	}
	else
	{
		hlog::write(april::logTag, "Aborting application quit request per app's request.");
	}	
#endif
}

-(void)showAprilAboutMenu
{
	NSString* copyright;
	copyright = getLocalizedString(@"MenuCopyright", @"");
	NSDictionary* aboutOptions = @{ @"Copyright": copyright };
	[NSApp orderFrontStandardAboutPanelWithOptions:aboutOptions];
}

@end

NSString* getApplicationName()
{
	NSDictionary* dict;
	NSString* appName = 0;
	appName = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:@"CFBundleDisplayName"];
	if (!appName || [appName length] == 0)
	{
		dict = (NSDictionary*) CFBundleGetInfoDictionary(CFBundleGetMainBundle());
		if (dict)
		{
			appName = [dict objectForKey: @"CFBundleName"];
		}
		if (![appName length])
		{
			appName = [[NSProcessInfo processInfo] processName];
		}
	}
	return appName;
}

static void setApplicationMenu()
{
	NSMenu* appleMenu;
	NSMenuItem* menuItem;
	NSString* title;
	NSString* appName;
	appName = getApplicationName();
	appleMenu = [[NSMenu alloc] initWithTitle:@""];
	title = [getLocalizedString(@"MenuAbout", @"About ") stringByAppendingString:appName];
	[appleMenu addItemWithTitle:title action:@selector(showAprilAboutMenu) keyEquivalent:@""];
	[appleMenu addItem:[NSMenuItem separatorItem]];
	title = [getLocalizedString(@"MenuHide", @"Hide ") stringByAppendingString:appName];
	[appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];
	title = getLocalizedString(@"MenuHideOthers", @"Hide Others");
	menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:title action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];
	title = getLocalizedString(@"MenuShowAll", @"Show All");
	[appleMenu addItemWithTitle:title action:@selector(unhideAllApplications:) keyEquivalent:@""];
	[appleMenu addItem:[NSMenuItem separatorItem]];
	title = [getLocalizedString(@"MenuQuit", @"Quit ") stringByAppendingString:appName];
	[appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];
	menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
	[menuItem setSubmenu:appleMenu];
	[[NSApp mainMenu] addItem:menuItem];
	[NSApp setAppleMenu:appleMenu];
	[appleMenu release];
	[menuItem release];
}

// Replacement for NSApplicationMain
static void CustomApplicationMain()
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	AprilAppDelegate* appDelegate;
	[[NSOperationQueue mainQueue] setMaxConcurrentOperationCount:1];
	[[NSOperationQueue currentQueue] setMaxConcurrentOperationCount:1];
	[AprilApplication sharedApplication];
#ifdef SDL_USE_CPS
	{
		CPSProcessSerNum PSN;
		/* Tell the dock about us */
		if (!CPSGetCurrentProcess(&PSN))
		{
			if (!CPSEnableForegroundOperation(&PSN, 0x03, 0x3C, 0x2C, 0x1103))
			{
				if (!CPSSetFrontProcess(&PSN))
				{
					[SDLApplication sharedApplication];
				}
			}
		}
	}
#endif
	NSMenu* menu = [[NSMenu alloc] init];
	[NSApp setMainMenu:menu];
	setApplicationMenu();
	appDelegate = [[AprilAppDelegate alloc] init];
	[NSApplication sharedApplication].delegate = appDelegate;
	[NSApp activateIgnoringOtherApps:YES];
	[NSApp run];
	[menu release];
	[appDelegate release];
	[pool release];
}

namespace april
{
	int __mainStandard(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), int argc, char** argv)
	{
		gAprilShouldInvokeQuitCallback = 0;
		harray<hstr> args;
		if (argc >= 2 && strncmp(argv[1], "-psn", 4) == 0)
		{
			args += argv[0];
			gFinderLaunch = YES;
		}
		else
		{
			if (argv != NULL && argv[0] != NULL)
			{
				for_iter (i, 0, argc)
				{
					args += argv[i];
				}
			}
			gFinderLaunch = NO;
		}
		april::application = new Application(aprilApplicationInit, aprilApplicationDestroy);
		april::application->setArgs(args);
		CustomApplicationMain();
		if (april::application != NULL)
		{
			delete april::application;
			april::application = NULL;
		}
		return april::getExitCode();
	}
	
}
