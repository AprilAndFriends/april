/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines all functions used in aprilui.
#import "ApriliOSAppDelegate.h"
#import "main.h"
#import "AprilViewController.h"
#import "EAGLView.h"
#include "RenderSystem.h"
#include "Window.h"
#import <AVFoundation/AVFoundation.h>

@implementation ApriliOSAppDelegate

@synthesize uiwnd;
@synthesize viewController;
@synthesize onPushRegistrationSuccess;
@synthesize onPushRegistrationFailure;

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	NSLog(@"Creating iOS window");
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];
	[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryAmbient error:NULL];

	// create a window.
	// early creation so Default.png can be displayed while we're waiting for 
	// game initialization
	uiwnd = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    uiwnd.autoresizesSubviews = YES;

	// viewcontroller will automatically add imageview
	viewController = [[AprilViewController alloc] init];
	
	if ([uiwnd respondsToSelector: @selector(rootViewController)])
		uiwnd.rootViewController = viewController; // only available on iOS4+, required on iOS6+
	else
		[uiwnd addSubview:viewController.view];

	// set window color
	[uiwnd setBackgroundColor:[UIColor blackColor]];
	
	april::Window::handleLaunchCallback(viewController);
	
	// display the window
	[uiwnd makeKeyAndVisible];
	//////////
	// thanks to Kyle Poole for this trick
    // also used in latest SDL
    // quote:
    // KP: using a selector gets around the "failed to launch application in time" if the startup code takes too long
    // This is easy to see if running with Valgrind
	[self performSelector:@selector(performInit:) withObject:nil afterDelay:0.2f];
    
}

- (void)performInit:(id)object
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    april_init(harray<hstr>());
	[pool drain];

	((EAGLView*) viewController.view)->app_started = 1;
	[(EAGLView*)viewController.view startAnimation];

	[pool release];
	
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	NSLog(@"Received iOS memory warning!");
	april::window->handleLowMemoryWarning();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    if (![[viewController.view subviews] count]) 
    {
        return;
    }
	
	april::window->handleFocusChangeEvent(0);
	
    for (EAGLView *glview in [viewController.view subviews])
    {
        if ([glview isKindOfClass:[EAGLView class]]) 
        {
            [glview stopAnimation];
            return;
        }
    }
    april_destroy();
}

- (NSUInteger)application:(UIApplication*)application supportedInterfaceOrientationsForWindow:(UIWindow*)window
{
	// this is a needed Hack to fix an iOS6 bug
	// more info: http://stackoverflow.com/questions/12488838/game-center-login-lock-in-landscape-only-in-i-os-6/12560069#12560069
    return UIInterfaceOrientationMaskAllButUpsideDown;
}

- (BOOL)application:(UIApplication*) application handleOpenURL:(NSURL *)url
{
	NSString* str = [url absoluteString];
	hstr urlstr = [str UTF8String];

	return april::window->handleUrl(urlstr) ? YES : NO;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    if (![[viewController.view subviews] count]) 
    {
        return;
    }

	if ([viewController.view isKindOfClass:[EAGLView class]]) 
	{
		EAGLView *glview = (EAGLView*)viewController.view;
		
		[glview applicationWillResignActive:application];
		[glview stopAnimation];

	}
	if ([[viewController.view subviews] count]) 
    {
		for (EAGLView* glview in viewController.view.subviews) 
		{
			if ([glview isKindOfClass:[EAGLView class]]) 
			{
				[glview applicationWillResignActive:application];
				[glview stopAnimation];
			}
		}
    }
	
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	// for our purposes, we don't need to differentiate entering background
	// from resigning activity
	[self applicationWillResignActive:application];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    if (![[viewController.view subviews] count]) 
    {
        return;
    }
	
	if ([viewController.view isKindOfClass:[EAGLView class]]) 
	{
		EAGLView *glview = (EAGLView*)viewController.view;
		
		[glview applicationDidBecomeActive:application];
		[glview startAnimation];
	}
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	// for our purposes, we don't need to differentiate entering foreground
	// from becoming active
	[self applicationDidBecomeActive:application];
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
	if (viewController)
	{
		[viewController release];
		viewController = nil;
	}
	self.window = nil;
}

@end
