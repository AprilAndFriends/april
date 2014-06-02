/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION

#ifndef APRIL_MAC_APP_DELEGATE_H
#define APRIL_MAC_APP_DELEGATE_H

#import <AppKit/NSApplication.h>

@interface AprilAppDelegate : NSObject<NSApplicationDelegate>
{
	bool mAppFocused;
}

- (void) applicationDidFinishLaunching: (NSNotification*) note;
- (void) applicationWillTerminate:(NSNotification*) note;
- (void)applicationDidBecomeActive:(NSNotification *)aNotification;
- (void)applicationDidResignActive:(NSNotification *)aNotification;

@end

#endif
