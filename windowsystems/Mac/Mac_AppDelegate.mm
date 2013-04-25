/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Foundation/Foundation.h>
#include <hltypes/hstring.h>
#import "Mac_AppDelegate.h"
#import "Mac_Window.h"

extern int gArgc;
extern char** gArgv;
extern void (*gAprilInit)(const harray<hstr>&);
extern void (*gAprilDestroy)();
NSString* getApplicationName();

@implementation AprilAppDelegate

- (void) receiveSleepNote: (NSNotification*) note
{
	if (aprilWindow && aprilWindow->isFocused())
	{
		aprilWindow->handleFocusChangeEvent(0);
		mNotifyFocusChanged = true;
	}
	else mNotifyFocusChanged = false;
}

- (void) receiveWakeNote: (NSNotification*) note
{
	if (mNotifyFocusChanged)
	{
		aprilWindow->handleFocusChangeEvent(true);
		mNotifyFocusChanged = false;
	}
}

- (void) applicationDidFinishLaunching: (NSNotification*) note
{
	mNotifyFocusChanged = false;

	harray<hstr> argv;
	for (int i = 0; i < gArgc; i++)
	{
		argv.push_back(gArgv[i]);
	}
	gAprilInit(argv);
	// register for sleep/wake notifications, needed for proper handling
	// of focus/unfocus events
	
	NSNotificationCenter* c = [[NSWorkspace sharedWorkspace] notificationCenter];
	[c addObserver:self selector: @selector(receiveSleepNote:) name:NSWorkspaceWillSleepNotification object:NULL];
	
    [c addObserver:self selector: @selector(receiveWakeNote:) name:NSWorkspaceDidWakeNotification object:NULL];

	
#ifdef _SDL
	april::window->enterMainLoop();
	gAprilDestroy();
	exit(0);
#endif
}

- (void) applicationWillTerminate:(NSNotification*) note
{
	gAprilDestroy();
}

- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
	aprilWindow->OnAppGainedFocus();
}
- (void)applicationDidResignActive:(NSNotification *)aNotification
{
	aprilWindow->OnAppLostFocus();
}

@end
