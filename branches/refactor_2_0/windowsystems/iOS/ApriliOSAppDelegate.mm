/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @version 1.31
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

@synthesize window;
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
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    window.autoresizesSubviews = YES;

	// viewcontroller will automatically add imageview
	viewController = [[AprilViewController alloc] init];
    [window addSubview:viewController.view];

	// set window color
	[window setBackgroundColor:[UIColor blackColor]];
	// display the window
	[window makeKeyAndVisible];
	
//	[(EAGLView*)viewController.view beginRender];

	//glClearColor(1, 1, 0, 1);
	//glClear(GL_COLOR_BUFFER_BIT);

/*
	april::Texture* tex = april::rendersys->loadTexture("data/loading_screen_iphone_hd.png");
	april::rendersys->setTexture(tex);
	gmat4 ident; ident.setIdentity();
	april::rendersys->setProjectionMatrix(ident);
	april::rendersys->setModelviewMatrix(ident);
	
	april::rendersys->drawTexturedQuad(grect(-1,1,2,-2), grect(0,0,0.9375f,0.625f));
	*/
	//[(EAGLView*)viewController.view swapBuffers];
	//delete tex;

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
	april::rendersys->getWindow()->handleLowMemoryWarning();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    if (![[viewController.view subviews] count]) 
    {
        return;
    }
	
	april::rendersys->getWindow()->handleFocusEvent(0);
	
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

- (BOOL) application:(UIApplication*) application handleOpenURL:(NSURL *)url
{
	NSString* str = [url absoluteString];
	hstr urlstr = [str UTF8String];

	return april::rendersys->getWindow()->handleURL(urlstr) ? YES : NO;
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
