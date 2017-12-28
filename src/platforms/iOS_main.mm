/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <UIKit/UIKit.h>
#include "ApriliOSAppDelegate.h"

#include "Application.h"
#include "main_base.h"

namespace april
{
	int __mainStandard(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), int argc, char** argv)
	{	
		harray<hstr> args;
		if (argv != NULL && argv[0] != NULL)
		{
			for_iter (i, 0, argc)
			{
				args += argv[i];
			}
		}
		april::application = new Application(aprilApplicationInit, aprilApplicationDestroy);
		april::application->setArgs(args);
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
		delete april::application;
		april::application = NULL;
		return result;
	}
	
}
