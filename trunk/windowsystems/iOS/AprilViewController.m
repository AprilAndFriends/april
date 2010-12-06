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

// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {	
	[super loadView];
	NSLog(@"%s", __PRETTY_FUNCTION__);

	
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
