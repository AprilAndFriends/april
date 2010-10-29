/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/

#import "AprilUIKitDelegate.h"
#import "main.h"


@implementation AprilUIKitDelegate

@synthesize window;

extern int(*april_RealMain)(int argc, char** argv);

- (void)runMain:(id)sender
{	
	// thanks to Kyle Poole for this trick
	char *argv[] = {"april_ios"};
	int status = april_RealMain (1, argv); //gArgc, gArgv);
	status;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];
	
	// thanks to Kyle Poole for this trick
	// also used in latest SDL
	// quote:
	// KP: using a selector gets around the "failed to launch application in time" if the startup code takes too long
	// This is easy to see if running with Valgrind

	[self performSelector:@selector(runMain:) withObject:nil afterDelay:0.2f];
}

@end
