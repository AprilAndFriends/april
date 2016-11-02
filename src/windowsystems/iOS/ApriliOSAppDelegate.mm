/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
#import "TargetConditionals.h"
#import "ApriliOSAppDelegate.h"
#import "main_base.h"
#import "AprilViewController.h"
#import "EAGLView.h"
#include "april.h"
#include <hltypes/hlog.h>
#include "iOS_Window.h"
#include "RenderSystem.h"
#include "Window.h"
#import <AVFoundation/AVFoundation.h>

typedef bool (*iOSUrlCallback)(chstr, chstr, void*);
static harray<iOSUrlCallback> gUrlCallbacks;
void april_iOS_registerUrlCallback(iOSUrlCallback ptr)
{
	gUrlCallbacks += ptr;
}

extern UIInterfaceOrientation gSupportedOrientations;

@implementation ApriliOSAppDelegate

@synthesize uiwnd;
@synthesize viewController;

- (UIWindow*)window
{
	return uiwnd;
}

- (BOOL)application:(UIApplication*) application didFinishLaunchingWithOptions:(NSDictionary*) launchOptions
{
	hlog::write(april::logTag, "Creating iOS window");
#if TARGET_IPHONE_SIMULATOR
	NSLog(@"[april] iOS Simulator document location: %@",[[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject]);
#endif
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];

    // figure out prefered app orientations
    NSArray* orientations = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];
    if (orientations != nil)
    {
        gSupportedOrientations = 0;
        for (NSString* orient in orientations)
        {
            if ([orient isEqual: @"UIInterfaceOrientationPortrait"])
            {
                gSupportedOrientations |= UIInterfaceOrientationMaskPortrait;
            }
            else if ([orient isEqual: @"UIInterfaceOrientationPortraitUpsideDown"])
            {
                gSupportedOrientations |= UIInterfaceOrientationMaskPortraitUpsideDown;
            }
            else if ([orient isEqual: @"UIInterfaceOrientationLandscapeLeft"])
            {
                gSupportedOrientations |= UIInterfaceOrientationMaskLandscapeLeft;
            }
            else if ([orient isEqual: @"UIInterfaceOrientationLandscapeRight"])
            {
                gSupportedOrientations |= UIInterfaceOrientationMaskLandscapeRight;
            }
        }
    }
	// create a window.
	// early creation so Default.png can be displayed while we're waiting for 
	// game initialization
	CGRect frame = getScreenBounds();
	uiwnd = [[UIWindow alloc] initWithFrame:frame];
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
	[self performSelector:@selector(performInit:) withObject:nil afterDelay:0.2f];
	
    return YES;
}

- (void)performInit:(id)object
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	//NSAutoreleasePool *pool = [NSAutoreleasePool new];
	harray<hstr> args;
	args += ""; // unable to determine executable name, but due to convention, leave one argument filled
	april_init(args);
	[pool drain];

	((EAGLView*) viewController.view)->app_started = 1;
	[(EAGLView*)viewController.view startAnimation];
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	hlog::write(april::logTag, "Received iOS memory warning!");
	april::window->handleLowMemoryWarningEvent();
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
    if (isiOS8OrNewer())
	{
        return gSupportedOrientations;
	}
    if (gSupportedOrientations == UIInterfaceOrientationMaskLandscape)
    {
        // this is a needed Hack to fix an iOS6 bug
        // more info: http://stackoverflow.com/questions/12488838/game-center-login-lock-in-landscape-only-in-i-os-6/12560069#12560069
        return UIInterfaceOrientationMaskAllButUpsideDown;
    }
    return gSupportedOrientations;
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
	NSString* str = [url absoluteString];
	hstr urlstr = [str UTF8String], srcAppStr = [sourceApplication UTF8String];
	
	BOOL ret = NO;
	bool r;
	foreach (iOSUrlCallback, it, gUrlCallbacks)
	{
		r = (*it)(urlstr, srcAppStr, annotation);
		if (r)
		{
			ret = YES;
		}
	}
	return ret;
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
