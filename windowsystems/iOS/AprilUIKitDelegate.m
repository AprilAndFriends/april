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
#import "AprilViewController.h"
#import "EAGLView.h"

@implementation AprilUIKitDelegate

@synthesize window;
@synthesize viewController;
@synthesize onPushRegistrationSuccess;
@synthesize onPushRegistrationFailure;
extern int(*april_RealMain)(int argc, char** argv);


- (void)runMain:(id)sender
{	
	// thanks to Kyle Poole for this trick
	char *argv[] = {"april_ios"};
	int status = april_RealMain (1, argv); //gArgc, gArgv);
#pragma unused(status)
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];
	
	// create a window.
	// early creation so Default.png can be displayed while we're waiting for 
	// game initialization
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

	// viewcontroller will automatically add imageview
	viewController = [[AprilViewController alloc] initWithWindow:window];
	[viewController loadView];
	
	// set window color
	[window setBackgroundColor:[UIColor blackColor]];

	// display the window
	[window makeKeyAndVisible];
	
	
	NSLog(@"Created window");
	//////////
	
	
	// thanks to Kyle Poole for this trick
	// also used in latest SDL
	// quote:
	// KP: using a selector gets around the "failed to launch application in time" if the startup code takes too long
	// This is easy to see if running with Valgrind

	[self performSelector:@selector(runMain:) withObject:nil afterDelay:0.2f];
}


- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	NSLog(@"April-based application received memory warning!");
}



- (void)applicationWillResignActive:(UIApplication *)application
{
	if ([viewController.view respondsToSelector:@selector(applicationWillResignActive:)]) {
		[(EAGLView*)viewController.view applicationWillResignActive:application];
	}
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	if ([viewController.view respondsToSelector:@selector(applicationDidBecomeActive:)]) {
		[(EAGLView*)viewController.view applicationDidBecomeActive:application];
	}
}



///////////////////////////
// utils and handlers for apps 
// that need push notifications
///////////////////////////
#pragma mark Push notifications

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{
	if(onPushRegistrationSuccess)
		onPushRegistrationSuccess(deviceToken);
}
- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
	if(onPushRegistrationFailure)
		onPushRegistrationFailure(error);
}


- (void)dealloc
{
	[super dealloc];
	self.window = nil;
}

@end
