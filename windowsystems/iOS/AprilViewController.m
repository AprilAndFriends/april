/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/

#import "AprilViewController.h"
#import "EAGLView.h"

@implementation AprilViewController

/*
 // The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if ((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil])) {
        // Custom initialization
    }
    return self;
}
*/


- (id)initWithWindow:(UIWindow*)w
{
	if(self=[super init])
	{
		window = w; // just assign, to avoid circular references		
	}
	return self;
}


- (CGImageRef)CGImageRotatedByAngle:(CGImageRef)imgRef angle:(CGFloat)angle
{
	CGFloat angleInRadians = angle * (M_PI / 180);
	CGFloat width = CGImageGetWidth(imgRef);
	CGFloat height = CGImageGetHeight(imgRef);
	
	CGRect imgRect = CGRectMake(0, 0, width, height);
	CGAffineTransform transform = CGAffineTransformMakeRotation(angleInRadians);
	CGRect rotatedRect = CGRectApplyAffineTransform(imgRect, transform);
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGContextRef bmContext = CGBitmapContextCreate(NULL,
												   rotatedRect.size.width,
												   rotatedRect.size.height,
												   8,
												   0,
												   colorSpace,
												   kCGImageAlphaPremultipliedFirst);
	CGContextSetAllowsAntialiasing(bmContext, FALSE);
	CGContextSetInterpolationQuality(bmContext, kCGInterpolationNone);
	CGColorSpaceRelease(colorSpace);
	CGContextTranslateCTM(bmContext,
						  +(rotatedRect.size.width/2),
						  +(rotatedRect.size.height/2));
	CGContextRotateCTM(bmContext, angleInRadians);
	CGContextTranslateCTM(bmContext,
						  -(rotatedRect.size.width/2),
						  -(rotatedRect.size.height/2));
	CGContextDrawImage(bmContext, CGRectMake(0, 0,
											 rotatedRect.size.width,
											 rotatedRect.size.height),
					   imgRef);
	
	CGImageRef rotatedImage = CGBitmapContextCreateImage(bmContext);
	CFRelease(bmContext);
	[(id)rotatedImage autorelease];
	
	return rotatedImage;
}



// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {	
	[super loadView];
	
	UIUserInterfaceIdiom idiom = UIUserInterfaceIdiomPhone;
	if ([[UIDevice currentDevice] respondsToSelector:@selector(userInterfaceIdiom:)]) 
	{
		idiom = [UIDevice currentDevice].userInterfaceIdiom;
	}
	else if([UIScreen mainScreen].bounds.size.height == 1024)
	{
		idiom = UIUserInterfaceIdiomPad;
	}
	
	NSString *defaultPngName = @"Default";
	if(idiom == UIUserInterfaceIdiomPad)
	{
		defaultPngName = @"Default-Landscape";
	}
	
	UIImage *image = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:defaultPngName ofType:@"png"] ];
	;
	if(idiom == UIUserInterfaceIdiomPhone)
	{
		// default.png is incorrectly rotated on iphone
		// this sadly doesnt work on <4.0:
		//image = [UIImage imageWithCGImage:image.CGImage scale:1 orientation:UIImageOrientationLeft];
		
		// hence we added rotation implementation from http://blog.coriolis.ch/2009/09/04/arbitrary-rotation-of-a-cgimage/
		// and we use it here:
		CGImageRef ir = [self CGImageRotatedByAngle:image.CGImage angle:90];
		image = [UIImage imageWithCGImage:ir];
		
	}
	UIImageView *iv = [[[[UIImageView alloc] initWithImage:image] autorelease] retain];

	// add animated spinner
#if 0
	// FIXME needs to be optional
	UIActivityIndicatorView *aiv = [[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite] autorelease];
	
	[aiv setCenter:CGPointMake(iv.bounds.size.width / 2 - aiv.bounds.size.width/2, 
							   iv.bounds.size.height - aiv.bounds.size.height)];
	[iv addSubview:aiv];
	[aiv startAnimating];
#endif
	
	self.view = iv;
	
	[window addSubview:iv];
	
	
	NSLog(@"Displayed iv");
}



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	NSLog(@"%s", __PRETTY_FUNCTION__);
}



// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft || interfaceOrientation == UIInterfaceOrientationLandscapeRight);
}


- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc {
    [super dealloc];
}


@end
