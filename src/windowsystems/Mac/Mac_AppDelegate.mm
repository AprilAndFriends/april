/// @file
/// @version 5.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Foundation/Foundation.h>

#include <hltypes/hstring.h>
#include <hltypes/hlog.h>

#import "Mac_AppDelegate.h"
#import "Mac_Window.h"
#include "Application.h"
#include "april.h"

#define MAC_WINDOW ((april::Mac_Window*)april::window)

bool gAppStarted = false;

NSString* getApplicationName();

bool g_WindowFocusedBeforeSleep = false;

@implementation AprilAppDelegate

- (void) receiveSleepNote: (NSNotification*) note
{
	if (april::window != NULL && april::window->isFocused())
	{
#ifdef _DEBUG
		hlog::write(april::logTag, "Computer went to sleep while app was focused.");
#endif
		MAC_WINDOW->onFocusChanged(false);
		g_WindowFocusedBeforeSleep = true;
	}
	else
	{
		g_WindowFocusedBeforeSleep = false;
	}
}

- (void) receiveWakeNote: (NSNotification*) note
{
	if (g_WindowFocusedBeforeSleep)
	{
#ifdef _DEBUG
		hlog::write(april::logTag, "Computer waked from sleep, focusing window.");
#endif
		MAC_WINDOW->onFocusChanged(true);
		g_WindowFocusedBeforeSleep = false;
	}
}

- (void) applicationDidFinishLaunching: (NSNotification*) note
{
	NSLog(@"april::applicationDidFinishLaunching");
	mAppFocused = true;	// register for sleep/wake notifications, needed for proper handling of focus/unfocus events
	april::application->init();
	if (april::window == NULL)
	{
		[NSApp terminate:nil];
		return;
	}
	NSNotificationCenter* c = [[NSWorkspace sharedWorkspace] notificationCenter];
	[c addObserver:self selector: @selector(receiveSleepNote:) name:NSWorkspaceWillSleepNotification object:NULL];
	[c addObserver:self selector: @selector(receiveWakeNote:) name:NSWorkspaceDidWakeNotification object:NULL];
	if (april::isUsingCVDisplayLink())
	{
		hmutex::ScopeLock lock(&MAC_WINDOW->renderThreadSyncMutex);
		gAppStarted = true;
	}
}

- (void) applicationWillTerminate:(NSNotification*) note
{
	if (april::isUsingCVDisplayLink())
	{
		hmutex::ScopeLock lock(&MAC_WINDOW->renderThreadSyncMutex);
		gAppStarted = false;
	}
	april::application->updateFinishing();
	april::application->destroy();
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
	if (mAppFocused)
	{
		return; // this blocks initial app focus call
	}
	mAppFocused = true;
#ifdef _DEBUG
	hlog::write(april::logTag, "Application activated.");
#endif
	if (april::window != NULL)
	{
		// TODOx - if this is not a system required name, it should be changed to properly follow the convention
		MAC_WINDOW->OnAppGainedFocus();
		april::application->resume();
	}
}

- (void)applicationDidResignActive:(NSNotification *)aNotification
{
	if (!mAppFocused)
	{
		return;
	}
	mAppFocused = false;
#ifdef _DEBUG
	hlog::write(april::logTag, "Application deactivated.");
#endif
	if (april::window != NULL)
	{
		// TODOx - if this is not a system required name, it should be changed to properly follow the convention
		MAC_WINDOW->OnAppLostFocus();
		april::application->suspend();
	}
}

@end
