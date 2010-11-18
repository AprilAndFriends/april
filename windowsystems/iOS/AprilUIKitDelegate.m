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
@synthesize onSuccess;
@synthesize onFailure;
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
	
	// create a window.
	// early creation so Default.png can be displayed while we're waiting for 
	// game initialization
	window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	
	NSString *defaultPngName = @"Default";
	if([UIScreen mainScreen].bounds.size.height == 1024)
	{
		defaultPngName = @"Default-Landscape";
	}
	
	UIImage *image = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:defaultPngName ofType:@"png"] ];
	UIImageView *iv = [[[[UIImageView alloc] initWithImage:image] autorelease] retain];
	if([UIScreen mainScreen].bounds.size.height == 1024)
	{
		iv.center = CGPointMake([UIScreen mainScreen].bounds.size.width/2, 
								[UIScreen mainScreen].bounds.size.height/2);
		
		iv.transform = CGAffineTransformRotate(iv.transform, 3.14159/2.);
		//iv.transform = CGAffineTransformTranslate(iv.transform, 240, 320);
	}
	

	
	[window addSubview:iv];

	
	// add animated spinner
#if 0
	// FIXME needs to be optional
	UIActivityIndicatorView *aiv = [[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite] autorelease];
	
	[aiv setCenter:CGPointMake(iv.bounds.size.width / 2 - aiv.bounds.size.width/2, 
							   iv.bounds.size.height - aiv.bounds.size.height)];
	[iv addSubview:aiv];
	[aiv startAnimating];
#endif
	
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

///////////////////////////
// utils and handlers for apps 
// that need push notifications
///////////////////////////
#pragma mark Push notifications

- (void)application:(UIApplication *)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData *)deviceToken
{
	if(onSuccess)
		onSuccess(deviceToken);
}
- (void)application:(UIApplication *)application didFailToRegisterForRemoteNotificationsWithError:(NSError *)error
{
	if(onFailure)
		onFailure(error);
}


- (void)dealloc
{
	[super dealloc];
	self.window = nil;
}

@end
