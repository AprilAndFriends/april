/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <UIKit/UIKit.h>
#include "ApriliOSAppDelegate.h"

#import "main_base.h"

int __april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv)
{	
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	// limit GCD from spawning too much threads
	[[NSOperationQueue mainQueue] setMaxConcurrentOperationCount:1];
	[[NSOperationQueue currentQueue] setMaxConcurrentOperationCount:1];
	int result = UIApplicationMain(argc, argv, nil, NSStringFromClass([ApriliOSAppDelegate class]));
    [pool release];
    return result;
}
