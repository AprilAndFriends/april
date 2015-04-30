/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView
{
	CGRect frame = getScreenBounds();
	glview = [[[EAGLView alloc] initWithFrame:frame] autorelease];
	self.view = glview;

	UIUserInterfaceIdiom idiom = UIUserInterfaceIdiomPhone;
	CGSize size = frame.size;
	if ([[UIDevice currentDevice] respondsToSelector:@selector(userInterfaceIdiom:)])
	{
		idiom = [UIDevice currentDevice].userInterfaceIdiom;
	}
	else if(size.width >= 768)
	{
		idiom = UIUserInterfaceIdiomPad;
	}
	
	// search for default launch image and make a view controller out of it to use while loading
	NSString *defaultPngName = @"";
	NSArray *allPngImageNames = [[NSBundle mainBundle] pathsForResourcesOfType:@"png" inDirectory:nil];
	hstr s;
	
	for (NSString* imgName in allPngImageNames)
	{
		s = [imgName UTF8String];
		if (s.contains("LaunchImage") || s.contains("Default"))
		{
			UIImage *img = [UIImage imageNamed:imgName];
			// Has image same scale and dimensions as our current device's screen?
			if (img.scale == [UIScreen mainScreen].scale && CGSizeEqualToSize(img.size, [UIScreen mainScreen].bounds.size))
			{
				hstr name = [imgName UTF8String];
				if (name.contains("/"))
				{
					name = name.rsplit("/", 1)[1];
				}
				NSLog(@"Found launch image for current device: %s: %@", name.cStr(), img.description);
				defaultPngName = imgName;
				break;
			}
		}
	}

	UIImage *image = [UIImage imageWithContentsOfFile:defaultPngName];

	if(idiom == UIUserInterfaceIdiomPhone && self.interfaceOrientation != UIInterfaceOrientationPortrait)
	{
		image = [UIImage imageWithCGImage:image.CGImage scale:1 orientation:UIImageOrientationRight];
	}
	
	mImageView = [[UIImageView alloc] initWithImage:image];
	if (isiOS8OrNewer())
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
