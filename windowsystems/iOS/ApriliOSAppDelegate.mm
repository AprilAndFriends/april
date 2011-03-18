/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/

#import "ApriliOSAppDelegate.h"
#import "main.h"
#import "AprilViewController.h"
#import "EAGLView.h"


@interface AprilDummyViewController : UIViewController
@end
@implementation AprilDummyViewController
- (void)loadView
{
    self.view = [[[UIView alloc] initWithFrame:CGRectMake(0, 0, 480, 320)] autorelease];
    self.view.autoresizesSubviews = YES;    
}
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
    //return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft || interfaceOrientation == UIInterfaceOrientationLandscapeRight);
    return YES;
}
@end



@implementation ApriliOSAppDelegate

@synthesize window;
@synthesize viewController;
@synthesize onPushRegistrationSuccess;
@synthesize onPushRegistrationFailure;

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];
    [[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft animated:NO];

	// create a window.
	// early creation so Default.png can be displayed while we're waiting for 
	// game initialization
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    window.autoresizesSubviews = YES;
    
    // add dummy view controller, so that shouldAutorotate is respected on old devices
    dummyViewController = [[UIViewController alloc] init];
    [window addSubview:dummyViewController.view];
    
	// viewcontroller will automatically add imageview
	viewController = [[[AprilViewController alloc] init] autorelease];
    [dummyViewController presentModalViewController:viewController animated:NO];
    [[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft animated:NO];

	// set window color
	[window setBackgroundColor:[UIColor blackColor]];

	// display the window
	[window makeKeyAndVisible];

	NSLog(@"Created window");
	//////////
	
	
    april_init(harray<hstr>());
}


- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	NSLog(@"April-based application received memory warning!");
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    april_destroy();
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
