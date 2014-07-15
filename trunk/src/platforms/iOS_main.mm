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

int april_main (void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char **argv)
{	
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, nil, NSStringFromClass([ApriliOSAppDelegate class]));
    [pool release];
    return retVal;
}
