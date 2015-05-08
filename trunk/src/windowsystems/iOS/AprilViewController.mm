/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
#include <hltypes/hlog.h>
#include "april.h"
#import "AprilViewController.h"
#import "EAGLView.h"
#import "WBImage.h"
#import "ApriliOSAppDelegate.h"
#include "iOS_Window.h"
#import <QuartzCore/CALayer.h>

extern EAGLView *glview;
static UIImageView *mImageView;
@implementation AprilViewController
bool g_wnd_rotating = 0;

UIInterfaceOrientation gSupportedOrientations = UIInterfaceOrientationMaskLandscape;

-(id)init
{
	self = [super init];
	self.wantsFullScreenLayout = YES;
	return self;
}
// helper function that scans through all png files in the root of an .app folder
// and tries to figure out which one is used as the launch image. The result is cached
// in NSUserDefaults so the search only needs to be performed on the initial startup
// Various tricks are used to predict which images are more likely to be the launch image
// in order to reduce the time it takes to scan through.
- (NSString*)getLaunchImageName:(UIUserInterfaceIdiom)idiom interfaceOrientation:(UIInterfaceOrientation)interfaceOrientation windowSize:(CGSize)wndSize
{
    // TODO: this code assumes app is either portrait-only or landscape-only
    // apps that support all orientations which have different Launch Images for
    // landscape and portait orientations will not experience correct behaviour.
    // In future, april-ios needs to see which orientation is currently active and
    // display that launch image as an overlay.

    NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
//    bool landscape = UIInterfaceOrientationIsLandscape(interfaceOrientation);
    NSString* nsDefaultPngName = [userDefaults objectForKey:@"april_LaunchImage"];
//#ifdef _DEBUG
//    hlog::write(logTag, "Debug: forcing launch image detection for debugging purposes");
//    nsDefaultPngName = nil;
//#endif
    if (nsDefaultPngName != nil)
    {
		hlog::writef(april::logTag, "Found cached LaunchImage: %s.png", [nsDefaultPngName UTF8String]);
        return nsDefaultPngName;
    }
    else
    {
		hlog::write(april::logTag, "Cached LaunchImage not found, detecting...");
    }

    hstr defaultPngName, rotatedPngName;
    NSArray *allPngImageNames = [[NSBundle mainBundle] pathsForResourcesOfType:@"png" inDirectory:nil];
    hstr s;
    bool iPadImage;
    harray<hstr> pnglist, primaryList, secondaryList;
    float screenScale = [UIScreen mainScreen].scale;
    CGSize screenSize = [UIScreen mainScreen].bounds.size;
    int screenHeight = screenSize.width > screenSize.height ? screenSize.width : screenSize.height;
    CGSize rotatedScreenSize = CGSizeMake(screenSize.height, screenSize.width);
    
    for (NSString* imgName in allPngImageNames)
    {
        s = [imgName UTF8String];
        iPadImage = s.contains("ipad");
        if (s.contains("LaunchImage") || s.contains("Default"))
        {
            // try to prioritize launch images for faster first startup
            if (idiom == UIUserInterfaceIdiomPhone && !iPadImage) // phone
            {
                if ((screenHeight == 736 && s.contains("736h")) ||
                    (screenHeight == 667 && s.contains("667h")) ||
                    (screenHeight == 568 && s.contains("568h")) ||
                    (screenHeight == 480 && screenScale == 2 && !s.contains("h@")) ||
                    (screenHeight == 480 && screenScale == 1 && !s.contains("@2x")))
                {
                    pnglist += s;
                }
                else
                {
                    primaryList += s;
                }
            }
            else if (idiom == UIUserInterfaceIdiomPad && iPadImage) // ipad
            {
                bool contains2x = s.contains("@2x");
                if ((screenScale == 1 && !contains2x) || (screenScale == 2 && contains2x))
                {
                    pnglist += s;
                }
                else
                {
                    primaryList += s;
                }
            }
            else
            {
                primaryList += s;
            }
        }
        else
        {
            // secondary list contains all other png images, if the user uses a non standard naming convention, search
            // these as well, but search them last
            secondaryList += s;
        }
    }

    pnglist += primaryList;
    pnglist += secondaryList;

    foreach (hstr, it, pnglist)
    {
        s = *it;
        if (s.contains("LaunchImage") || s.contains("Default"))
        {
            //UIImage *img = [UIImage imageNamed:imgName];
            UIImage *img = [UIImage imageWithContentsOfFile:[NSString stringWithUTF8String:s.cStr()]];
            
            // Has image same scale and dimensions as our current device's screen?
            if (img.scale == screenScale && (CGSizeEqualToSize(img.size, screenSize) || CGSizeEqualToSize(img.size, rotatedScreenSize)))
            {
                if (s.contains("/"))
                {
                    s = s.rsplit("/", 1)[1].replaced(".png", "");
                }
                NSLog(@"Found launch image for device: %s.png: %@", s.cStr(), img.description);
                defaultPngName = s;
                break;
            }
        }
    }
    if (defaultPngName == "")
    {
        NSLog(@"Failed to find appropriate launch image for device");
        return @"";
    }
    else
    {
        nsDefaultPngName = [NSString stringWithUTF8String:defaultPngName.cStr()];
        [userDefaults setObject:nsDefaultPngName forKey:@"april_LaunchImage"];
        [userDefaults synchronize];
        return nsDefaultPngName;
    }
}

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView
{
	CGRect frame = getScreenBounds();
	glview = [[[EAGLView alloc] initWithFrame:frame] autorelease];
	self.view = glview;

	UIUserInterfaceIdiom idiom = UIUserInterfaceIdiomPhone;
	CGSize size = frame.size;
	idiom = [UIDevice currentDevice].userInterfaceIdiom;
	
	// search for default launch image and make a view controller out of it to use while loading
    NSString* defaultPngName = [self getLaunchImageName:idiom interfaceOrientation:self.interfaceOrientation windowSize:size];
    UIImage* image = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:defaultPngName ofType:@"png"]];

	if(idiom == UIUserInterfaceIdiomPhone && self.interfaceOrientation != UIInterfaceOrientationPortrait)
	{
		image = [UIImage imageWithCGImage:image.CGImage scale:1 orientation:UIImageOrientationRight];
	}
	
	mImageView = [[UIImageView alloc] initWithImage:image];
	if (isiOS8OrNewer() || UIInterfaceOrientationIsPortrait(self.interfaceOrientation))
	{
		mImageView.frame = CGRectMake(0, 0, self.view.bounds.size.width, self.view.bounds.size.height);
	}
	else
	{
		mImageView.frame = CGRectMake(0, 0, self.view.bounds.size.height, self.view.bounds.size.width);
	}
	[self.view addSubview:mImageView];
	mImageView.layer.zPosition = 1;
}

- (void)animationWillStart:(NSString*)animationID context:(void*)context
{
	
}

- (void) animationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context
{
	if ([animationID isEqual: @"FadeOut"])
	{
		NSLog(@"Removing loading screen UIImageView from View Controller");
		[mImageView removeFromSuperview];
		[mImageView release];
		mImageView = nil;
	}
}


- (void)removeImageView:(bool)fast
{
	if (mImageView != nil)
	{
		NSLog(@"Performing fadeout of loading screen's UIImageView");

		[UIView beginAnimations:@"FadeOut" context:nil];
		[UIView setAnimationDuration:(fast ? 0.25f : 1)];
		mImageView.alpha = 0;
		[UIView  setAnimationDelegate:self];
		[UIView setAnimationDidStopSelector:@selector(animationDidStop:finished:context:)];
		[UIView commitAnimations];
	}
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
	[super viewDidLoad];
}

-(void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
	NSLog(@"Window started rotating");
	g_wnd_rotating = 1;
}

-(void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	NSLog(@"Window finished rotating");
	g_wnd_rotating = 0;
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation // used in iOS versions older than iOS6
{
    return (gSupportedOrientations & (1 << interfaceOrientation)) != 0;
}

-(NSUInteger) supportedInterfaceOrientations // used in iOS6+ only
{
    return gSupportedOrientations;
}

- (void)didReceiveMemoryWarning
{
	// Releases the view if it doesn't have a superview.
	[super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)dealloc
{
	[super dealloc];
}


@end
