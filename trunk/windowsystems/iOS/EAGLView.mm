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

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "EAGLView.h"
#include "iOSWindow.h"
#include "RenderSystem.h"
#include "april.h"
#include "Keys.h"


#define USE_DEPTH_BUFFER 0
#define aprilWindow april::iOSWindow::getSingleton()


// A class extension to declare private methods
@interface EAGLView ()

@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) NSTimer *animationTimer;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end



@implementation EAGLView

@synthesize context;
@synthesize animationTimer;
//@synthesize animationInterval;
@synthesize aprilWindowVoid;

// You must implement this method
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}


// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	aprilWindow->touchesBegan_withEvent_(touches, event);
}

// Handles the end of a touch event when the touch is a tap.
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	aprilWindow->touchesEnded_withEvent_(touches, event);
}

// Called if touches are cancelled and need to be undone. 
// On iPhone, happens when 5 fingers are pressed.
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	aprilWindow->touchesCancelled_withEvent_(touches, event);
}

// Handles the movement of a touch event. 
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	aprilWindow->touchesMoved_withEvent_(touches, event);
}

- (id)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame]))
	{
		app_started = 0;
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		if ([eaglLayer respondsToSelector:@selector(setContentsScale:)])
		{
			if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
			{ // iphone 4
				eaglLayer.contentsScale = [[UIScreen mainScreen] scale];
			}
		}
#endif
		self.multipleTouchEnabled = YES;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context])
		{
            [self release];
            return nil;
        }
        
        animationInterval = 1.0 / 60.0;
		
		
		// fake textbox
		CGRect textFrame = frame;
		textFrame.origin.x += frame.size.width;
		textField = [[UITextField alloc] initWithFrame:textFrame];
		textField.delegate = self;
		textField.text = @" "; // something to be able to catch text edit
		textField.autocapitalizationType = UITextAutocapitalizationTypeNone;
		textField.autocorrectionType = UITextAutocorrectionTypeNo;
		textField.keyboardAppearance = UIKeyboardAppearanceDefault;
		textField.keyboardType = UIKeyboardTypeDefault;
		textField.returnKeyType = UIReturnKeyDone;
		textField.secureTextEntry = NO;
		[self addSubview:textField];

		// tracking of keyboard appearance/disappearance
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(keyboardWasShown:)
													 name:UIKeyboardDidShowNotification object:nil];
		
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(keyboardWasHidden:)
													 name:UIKeyboardDidHideNotification object:nil];
		// tracking of device orientation change
		// we call UIDevice's beginGeneratingDeviceOrientationNotifications
		// in window itself 
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(deviceOrientationDidChange:)
													 name:UIDeviceOrientationDidChangeNotification 
												   object:nil];
		
		[textField addTarget:self
						   action:@selector(textFieldFinished:)
							forControlEvents:UIControlEventEditingDidEndOnExit];
		// create ios window object
		april::createRenderSystem("create_eagl");
		april::rendersys->assignWindow(new april::iOSWindow(0,0,1,"iOS Window"));
    }
	
    return self;
}

- (BOOL)textFieldShouldReturn:(UITextField *)aTextField
{
    [textField resignFirstResponder];
	aprilWindow->handleKeyEvent(april::Window::AKEYEVT_DOWN, april::AK_RETURN, 0);
	aprilWindow->handleKeyEvent(april::Window::AKEYEVT_UP, april::AK_RETURN, 0);
    return YES;
}

// we'll also use this objc class for getting notifications
// on virtual keyboard's appearance and disappearance

- (void)keyboardWasShown:(id)sender
{
	aprilWindow->keyboardWasShown();
}

- (void)keyboardWasHidden:(id)sender
{
	aprilWindow->keyboardWasHidden();
}

// we'll also use this objc class for getting notifications
// when device orientation changes

-(void)deviceOrientationDidChange:(id)sender
{
	aprilWindow->deviceOrientationDidChange();
}

// ok, now other functionality of this class

- (void)beginRender
{
    if (!self.animationTimer)
	{
		NSLog(@"Called drawView while in background!");
		return;
	}
 //   glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer); // commented this out on June 8th 2012, it's probably reduntant, but I'll keep it here for a while just in case. -- kspes
    
    [EAGLContext setCurrentContext:context];
}

- (void)drawView
{
    [self beginRender];
	
	//mydraw();
	((april::iOSWindow*)aprilWindow)->handleDisplayAndUpdate();
	


    //[self swapBuffers];
}


-(void)_paintRect:(GLfloat[])vertices
{
	
}


- (void)swapBuffers
{
	if (!self.animationTimer)
	{
		NSLog(@"Warning: OpenGL swapBuffers while app in background, ignoring!");
		return;
	}
//	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer); // commented this out on June 8th 2012, it's probably reduntant, but I'll keep it here for a while just in case. -- kspes
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];

}

- (void)layoutSubviews
{
	CGRect textFrame = textField.frame;
	textFrame.origin.x += textFrame.size.width;
	textField.frame = textFrame;
	
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];
}


- (BOOL)createFramebuffer
{
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
	
	NSString *depth = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"april_depth_buffer"];
	
    if (depth != nil)
	{
        glGenRenderbuffersOES(1, &depthRenderbuffer);
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
        glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
        glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    }
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
	// clear crap from previous renders. I often got a magenta colored initial screen without this
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, backingWidth, backingHeight);
    return YES;
}


- (void)destroyFramebuffer
{
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
    
    if(depthRenderbuffer)
	{
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}


- (void)startAnimation
{
	if (!app_started) return;
    self.animationTimer = [NSTimer scheduledTimerWithTimeInterval:animationInterval target:self selector:@selector(drawView) userInfo:nil repeats:YES];
}


- (void)stopAnimation
{
    self.animationTimer = nil;
}


- (void)setAnimationTimer:(NSTimer *)newTimer
{
    if(animationTimer != newTimer)
		[animationTimer invalidate];
    animationTimer = newTimer;
}


- (void)setAnimationInterval:(NSTimeInterval)interval
{
    
    animationInterval = interval;
    if (animationTimer)
	{
        [self stopAnimation];
        [self startAnimation];
    }
}

- (NSTimeInterval)setAnimationInterval
{
	return animationInterval;
}

- (void)dealloc
{
    
    [self stopAnimation];
    
    if ([EAGLContext currentContext] == context)
	{
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

- (void)beginKeyboardHandling
{
	[textField becomeFirstResponder];
}

- (BOOL)isKeyboardActive
{
	return [textField isFirstResponder];
}

- (void)terminateKeyboardHandling
{
	[textField endEditing:YES];
}

- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string   // return NO to not change text
{
	hstr str = chstr([string UTF8String]);
	if (range.location == 0 && str.size() == 0) aprilWindow->injectiOSChar(0); // backspace indicator
	else if (str.size() == 0) return NO;
	unichar chars[256];
	[string getCharacters:chars];
	int len = [string length];
	
	for (int i = 0; i < len; i++)
	{
		aprilWindow->injectiOSChar(chars[i]);
	}
	return NO;
}

-(void)applicationDidBecomeActive:(UIApplication*)app
{
	aprilWindow->applicationDidBecomeActive();
}

-(void)applicationWillResignActive:(UIApplication*)app
{
	aprilWindow->applicationWillResignActive();
}

@end
