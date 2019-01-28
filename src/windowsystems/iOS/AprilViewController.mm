/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _IOS_WINDOW
#import <QuartzCore/CALayer.h>

#include <hltypes/hlog.h>

#import "AprilViewController.h"
#import "EAGLView.h"
#import "WBImage.h"
#import "ApriliOSAppDelegate.h"
#include "april.h"
#include "iOS_Window.h"

extern EAGLView* glview;
static UIImageView* mImageView = NULL;

@implementation AprilViewController

UIInterfaceOrientationMask gSupportedOrientations = UIInterfaceOrientationMaskLandscape;

-(id)init
{
	self = [super init];
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
	NSArray *allPngImageNames = [[NSBundle mainBundle] pathsForResourcesOfType:@"png" inDirectory:nil];
    if (nsDefaultPngName != nil)
    {
		bool found = false;
		hstr s, defaultImg = [nsDefaultPngName UTF8String] + hstr(".png");
		for (NSString* imgName in allPngImageNames)
		{
			s = [imgName UTF8String];
			if (s.endsWith(defaultImg))
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			hlog::writef(april::logTag, "Found cached Launch Image: %s.png", [nsDefaultPngName UTF8String]);
			return nsDefaultPngName;
		}
		else
		{
			hlog::writef(april::logTag, "Invalid cached Launch Image: %s.png! Deleting cached value and searching again.", [nsDefaultPngName UTF8String]);
			nsDefaultPngName = nil;
		}
    }
    else
    {
		hlog::write(april::logTag, "Cached Launch Image not found, detecting...");
    }
	hstr defaultPngName;
	hstr rotatedPngName;
    hstr s;
    bool iPadImage;
    harray<hstr> pnglist, secondaryList;
    float screenScale = [UIScreen mainScreen].scale;
    CGSize screenSize = [UIScreen mainScreen].bounds.size;
	CGSize originalScreenSize = screenSize;
    int screenHeight = screenSize.width > screenSize.height ? screenSize.width : screenSize.height;
    CGSize rotatedScreenSize = CGSizeMake(screenSize.height, screenSize.width);
	if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone)
	{
		// prefer vertical images on the iphone
		CGSize temp = screenSize;
		screenSize = rotatedScreenSize;
		originalScreenSize = rotatedScreenSize;
		rotatedScreenSize = temp;
	}
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
                    secondaryList += s;
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
                    secondaryList += s;
                }
            }
            else
            {
                secondaryList += s;
            }
        }
    }
    pnglist += secondaryList;
	hmap<hstr, gvec2f> imageProps;
	for_iter (i, 0, 2)
	{
		foreach (hstr, it, pnglist)
		{
			s = (*it);
			UIImage *img = [UIImage imageWithContentsOfFile:[NSString stringWithUTF8String:s.cStr()]];
			if (s.contains("/"))
			{
				s = s.rsplit("/", 1)[1].replaced(".png", "");
			}
			imageProps[s] = gvec2f(img.size.width, img.size.height);
			// Has image same scale and dimensions as our current device's screen?
			if (img.scale == screenScale && (CGSizeEqualToSize(img.size, screenSize)))
			{
				hlog::writef(april::logTag, "Found launch image for device: %s.png", s.cStr());
				defaultPngName = s;
				break;
			}
		}
		if (defaultPngName != "")
		{
			break;
		}
		screenSize = rotatedScreenSize; // just in case, do another pass
	}
    if (defaultPngName == "")
    {
		// assuming all the xcassets files are correct, this can happen on scaled screens on iphone6 and 6+
		// so let's try to find the most fitting image
		hlog::write(april::logTag, "Failed to find exact launch image for device, trying search by nearest aspect ratio");
		screenSize = originalScreenSize;
		gvec2f size;
		float aspect = 0.0f;
		float diff = 0.0f;
		float bestFit = 0.0f;
		float screenDiagonal = 0.0f;
		for_iter (i, 0, 2)
		{
			bestFit = 10000.0f;
			aspect = screenSize.width / screenSize.height;
			screenDiagonal = gvec2f(screenSize.width, screenSize.height).squaredLength();
			foreach_m (gvec2f, it, imageProps)
			{
				s = it->first;
				size = it->second;
				if (habs(size.x / size.y - aspect) < 0.001f)
				{
					diff = habs(screenDiagonal - size.squaredLength());
					if (diff < bestFit)
					{
						bestFit = diff;
						defaultPngName = s;
					}
				}
			}
			if (defaultPngName != "")
			{
				break;
			}
			screenSize = rotatedScreenSize; // just in case, do another pass
		}
		if (defaultPngName != "")
		{
			hlog::writef(april::logTag, "Found aproximate launch image: %s", defaultPngName.cStr());
		}
		else
		{
			hlog::write(april::logTag, "Failed to find aproximate launch image for device");
			return @"";
		}
    }
	nsDefaultPngName = [NSString stringWithUTF8String:defaultPngName.cStr()];
	[userDefaults setObject:nsDefaultPngName forKey:@"april_LaunchImage"];
	[userDefaults synchronize];
	return nsDefaultPngName;
}

-(UIInterfaceOrientation)getDeviceOrientation
{
	return [[UIApplication sharedApplication] statusBarOrientation];
}

// Implement loadView to create a view hierarchy programmatically, without using a nib.
-(void)loadView
{
	CGRect frame = getScreenBounds();
	glview = [[[EAGLView alloc] initWithFrame:frame] autorelease];
	self.view = glview;
	UIUserInterfaceIdiom idiom = UIUserInterfaceIdiomPhone;
	CGSize size = frame.size;
	idiom = [UIDevice currentDevice].userInterfaceIdiom;
	// search for default launch image and make a view controller out of it to use while loading
    NSString* defaultPngName = [self getLaunchImageName:idiom interfaceOrientation:[self getDeviceOrientation] windowSize:size];
	UIImage* image;
	if ([defaultPngName isEqualToString:@""])
	{
		// make solid color launch screen image
		CGRect rect = CGRectMake(0.0f, 0.0f, 1.0f, 1.0f);
		UIGraphicsBeginImageContext(rect.size);
		CGContextRef context = UIGraphicsGetCurrentContext();
		CGContextSetFillColorWithColor(context, [[UIColor blackColor] CGColor]);
		CGContextFillRect(context, rect);
		image = UIGraphicsGetImageFromCurrentImageContext();
		UIGraphicsEndImageContext();
	}
	else
	{
		image = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:defaultPngName ofType:@"png"]];
		if(idiom == UIUserInterfaceIdiomPhone && [self getDeviceOrientation] != UIInterfaceOrientationPortrait)
		{
			image = [UIImage imageWithCGImage:image.CGImage scale:1 orientation:UIImageOrientationRight];
		}
	}
	mImageView = [[UIImageView alloc] initWithImage:image];
	mImageView.frame = CGRectMake(0, 0, self.view.bounds.size.width, self.view.bounds.size.height);
	[self.view addSubview:mImageView];
	mImageView.layer.zPosition = 1;
}

// it's a nice idea, but the status color doesn't automatically adapt without hacking in some weird additional view
/*
-(BOOL)prefersStatusBarHidden
{
	// turn on status bar on iOS 11+ and iPhone X*
	if ([[[UIDevice currentDevice] systemVersion] compare:@"11.0" options:NSNumericSearch] == NSOrderedDescending)
	{
		UIEdgeInsets insets = [[[UIApplication sharedApplication] window] safeAreaInsets];
		if (insets.left != 0.0f || insets.right != 0.0f || insets.top != 0.0f || insets.bottom != 0.0f)
		{
			return YES;
		}
	}
	return [super prefersStatusBarHidden];
}
*/

-(void)animationWillStart:(NSString*)animationID context:(void*)context
{
}

-(void)animationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context
{
	if ([animationID isEqual: @"FadeOut"])
	{
		hlog::write(april::logTag, "Removing loading screen UIImageView from View Controller");
		[mImageView removeFromSuperview];
		[mImageView release];
		mImageView = nil;
	}
}

-(void)removeImageView:(bool)fast
{
	if (mImageView != nil)
	{
		hlog::write(april::logTag, "Performing fadeout of loading screen's UIImageView");
		[UIView beginAnimations:@"FadeOut" context:nil];
		[UIView setAnimationDuration:(fast ? 0.25f : 1)];
		mImageView.alpha = 0;
		[UIView setAnimationDelegate:self];
		[UIView setAnimationDidStopSelector:@selector(animationDidStop:finished:context:)];
		[UIView commitAnimations];
	}
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
-(void)viewDidLoad
{
	[super viewDidLoad];
}

-(NSUInteger)supportedInterfaceOrientations // used in iOS6+ only
{
    return gSupportedOrientations;
}

-(void)didReceiveMemoryWarning
{
	// Releases the view if it doesn't have a superview.
	[super didReceiveMemoryWarning];
	// Release any cached data, images, etc that aren't in use.
}

-(void)dealloc
{
	[super dealloc];
}

@end
#endif
