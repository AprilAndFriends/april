/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <UIKit/UIKit.h>
#include "ApriliOSAppDelegate.h"

#import "main_base.h"

namespace april
{
	extern harray<hstr> args;
}

int __april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{	
	if (argv != NULL && argv[0] != NULL)
	{
		for_iter (i, 0, argc)
		{
			april::args += argv[i];
		}
	}
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	// limit GCD from spawning too much threads
	[[NSOperationQueue mainQueue] setMaxConcurrentOperationCount:1];
	[[NSOperationQueue currentQueue] setMaxConcurrentOperationCount:1];
	
	NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
	NSString* appDelegateClassName = [userDefaults objectForKey:@"appDelegateClassName"];
	if (appDelegateClassName == nil)
	{
		appDelegateClassName = NSStringFromClass([ApriliOSAppDelegate class]);
	}
	int result = UIApplicationMain(argc, argv, nil, appDelegateClassName);
	[pool drain];
	return result;
}
