/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _IOS_WINDOW
#import <AVFoundation/AVFoundation.h>
#import <TargetConditionals.h>
#include <locale.h>

#include <hltypes/hlog.h>

#import "ApriliOSAppDelegate.h"
#import "main_base.h"
#import "AprilViewController.h"
#import "EAGLView.h"
#include "Application.h"
#include "april.h"
#include "iOS_Window.h"
#include "RenderSystem.h"
#include "Window.h"

typedef bool (*iOSUrlCallback)(chstr, chstr, void*);
static harray<iOSUrlCallback> gUrlCallbacks;
void april_iOS_registerUrlCallback(iOSUrlCallback ptr)
{
	gUrlCallbacks += ptr;
}

extern UIInterfaceOrientationMask gSupportedOrientations;

@implementation ApriliOSAppDelegate

@synthesize uiwnd;
@synthesize viewController;

- (UIWindow*)window
{
	return uiwnd;
}

- (BOOL)application:(UIApplication*) application didFinishLaunchingWithOptions:(NSDictionary*) launchOptions
{
	// this doesn't seem to work on iOS, but let's leave it here if Apple ever decides to actually support basic OS stuff
	setlocale(LC_ALL, __APRIL_DEFAULT_LOCALE); // make sure the app uses a neutral locale that includes all specifics for all locales
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
	uiwnd.rootViewController = viewController;
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
	((EAGLView*)viewController.view)->app_started = 1;
	[(EAGLView*)viewController.view startAnimation];
	april::application->init();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	hlog::write(april::logTag, "Received iOS memory warning!");
	april::window->queueLowMemoryWarning();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	if (![[viewController.view subviews] count]) 
	{
		return;
	}
	april::window->queueFocusChange(false);
	for (EAGLView* glview in [viewController.view subviews])
	{
		if ([glview isKindOfClass:[EAGLView class]]) 
		{
			[glview stopAnimation];
			return;
		}
	}
	april::application->finish();
	april::application->updateFinishing();
}

- (NSUInteger)application:(UIApplication*)application supportedInterfaceOrientationsForWindow:(UIWindow*)window
{
	return gSupportedOrientations;
}

- (BOOL)application:(UIApplication*)application openURL:(NSURL*)url sourceApplication:(NSString*)sourceApplication annotation:(id)annotation
{
	NSString* str = [url absoluteString];
	hstr urlstr = [str UTF8String], srcAppStr = [sourceApplication UTF8String];
	BOOL result = NO;
	foreach (iOSUrlCallback, it, gUrlCallbacks)
	{
		if ((*it)(urlstr, srcAppStr, annotation))
		{
			result = YES;
		}
	}
	return result;
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
	if (viewController != nil)
	{
		[viewController release];
		viewController = nil;
	}
	if (uiwnd != nil)
	{
		[uiwnd release];
		uiwnd = nil;
	}
	self.window = nil;
	[super dealloc];
}

@end
#endif
