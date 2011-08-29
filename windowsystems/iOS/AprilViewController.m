/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 * Copyright (c) 2010 Kresimir Spes (kspes@cateia.com), Ivan Vucica (ivan@vucica.net) *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/

#import "AprilViewController.h"
#import "EAGLView.h"
#import "WBImage.h"
#import "ApriliOSAppDelegate.h"

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


#pragma mark -
#pragma mark Portions of WBImage
// using wbimage category did not work for some reason

static inline CGFloat degreesToRadians(CGFloat degrees)
{
    return M_PI * (degrees / 180.0);
}

static inline CGSize swapWidthAndHeight(CGSize size)
{
    CGFloat  swap = size.width;
	
    size.width  = size.height;
    size.height = swap;
	
    return size;
}

-(UIImage*)rotate:(UIImage*)src to:(UIImageOrientation)orient
{
    CGRect             bnds = CGRectZero;
    UIImage*           copy = nil;
    CGContextRef       ctxt = nil;
    CGRect             rect = CGRectZero;
    CGAffineTransform  tran = CGAffineTransformIdentity;
	
    bnds.size = src.size;
    rect.size = src.size;
	
    switch (orient)
    {
        case UIImageOrientationUp:
			return src;
			
        case UIImageOrientationUpMirrored:
			tran = CGAffineTransformMakeTranslation(rect.size.width, 0.0);
			tran = CGAffineTransformScale(tran, -1.0, 1.0);
			break;
			
        case UIImageOrientationDown:
			tran = CGAffineTransformMakeTranslation(rect.size.width,
													rect.size.height);
			tran = CGAffineTransformRotate(tran, degreesToRadians(180.0));
			break;
			
        case UIImageOrientationDownMirrored:
			tran = CGAffineTransformMakeTranslation(0.0, rect.size.height);
			tran = CGAffineTransformScale(tran, 1.0, -1.0);
			break;
			
        case UIImageOrientationLeft:
			bnds.size = swapWidthAndHeight(bnds.size);
			tran = CGAffineTransformMakeTranslation(0.0, rect.size.width);
			tran = CGAffineTransformRotate(tran, degreesToRadians(-90.0));
			break;
			
        case UIImageOrientationLeftMirrored:
			bnds.size = swapWidthAndHeight(bnds.size);
			tran = CGAffineTransformMakeTranslation(rect.size.height,
													rect.size.width);
			tran = CGAffineTransformScale(tran, -1.0, 1.0);
			tran = CGAffineTransformRotate(tran, degreesToRadians(-90.0));
			break;
			
        case UIImageOrientationRight:
			bnds.size = swapWidthAndHeight(bnds.size);
			tran = CGAffineTransformMakeTranslation(rect.size.height, 0.0);
			tran = CGAffineTransformRotate(tran, degreesToRadians(90.0));
			break;
			
        case UIImageOrientationRightMirrored:
			bnds.size = swapWidthAndHeight(bnds.size);
			tran = CGAffineTransformMakeScale(-1.0, 1.0);
			tran = CGAffineTransformRotate(tran, degreesToRadians(90.0));
			break;
			
        default:
			// orientation value supplied is invalid
			assert(false);
			return nil;
    }
	
    UIGraphicsBeginImageContext(bnds.size);
    ctxt = UIGraphicsGetCurrentContext();
	
    switch (orient)
    {
        case UIImageOrientationLeft:
        case UIImageOrientationLeftMirrored:
        case UIImageOrientationRight:
        case UIImageOrientationRightMirrored:
			CGContextScaleCTM(ctxt, -1.0, 1.0);
			CGContextTranslateCTM(ctxt, -rect.size.height, 0.0);
			break;
			
        default:
			CGContextScaleCTM(ctxt, 1.0, -1.0);
			CGContextTranslateCTM(ctxt, 0.0, -rect.size.height);
			break;
    }
	
    CGContextConcatCTM(ctxt, tran);
    CGContextDrawImage(ctxt, rect, src.CGImage);
	
    copy = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
	
    return copy;
}




#pragma mark End WBImage portions
#pragma mark -



// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView
{	
    	
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

	if(idiom == UIUserInterfaceIdiomPhone && self.interfaceOrientation != UIInterfaceOrientationPortrait)
	{
		if ([UIImage instancesRespondToSelector:@selector(initWithCGImage:scale:orientation:)]) 
		{
			// default.png is incorrectly rotated on iphone
			// this sadly doesnt work on <4.0:
			
			// if UIImage responds to instance method, it will respond to the class method too.
			image = [UIImage imageWithCGImage:image.CGImage scale:1 orientation:UIImageOrientationRight];
		}
		else
		{
			// hence we added rotation implementation using WBImage by Allen Brunson and Kevin Lohman:
			// http://www.platinumball.net/blog/2010/01/31/iphone-uiimage-rotation-and-scaling/
			image = [self rotate:image to:UIImageOrientationRight]; // for some reason using WBImage category of UIImage did not work! code therefore copypasted to this class and called via self
		}
	}

	UIImageView *iv = [[[UIImageView alloc] initWithImage:image] autorelease];
	
	iv.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    iv.autoresizesSubviews = YES;
    self.view = iv;
    
    
	self.view.transform = CGAffineTransformMakeRotation(M_PI / 2.0); 
	[self.view setCenter:[[(ApriliOSAppDelegate*)[[UIApplication sharedApplication] delegate] window] center]];

}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
}

// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return (interfaceOrientation == UIInterfaceOrientationLandscapeLeft || interfaceOrientation == UIInterfaceOrientationLandscapeRight);
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)dealloc
{
    [super dealloc];
}


@end
