/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines all functions used in aprilui.

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#include <hltypes/hlog.h>

#import "EAGLView.h"
#include "Application.h"
#include "april.h"
#include "iOS_Window.h"
#include "Keys.h"
#include "MotionDelegate.h"
#include "RenderSystem.h"

#define USE_DEPTH_BUFFER 0
#define GRAVITY 9.81f
#define aprilWindow ((april::iOS_Window*)april::window)

// A class extension to declare private methods
@interface EAGLView ()

@property (nonatomic, retain) EAGLContext* context;
@property (nonatomic, assign) CADisplayLink* displayLink;
@property (nonatomic, assign) CMMotionManager* sensorManager;

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end

@implementation EAGLView

@synthesize context;
@synthesize displayLink;
@synthesize sensorManager;

// You must implement this method
+ (Class)layerClass
{
	return [CAEAGLLayer class];
}


// Handles the start of a touch
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window != NULL)
	{
		aprilWindow->touchesBegan_withEvent_(touches, event);
	}
}

// Handles the end of a touch event when the touch is a tap.
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window != NULL)
	{
		aprilWindow->touchesEnded_withEvent_(touches, event);
	}
}

// Called if touches are cancelled and need to be undone. 
// On iPhone, happens when 5 fingers are pressed.
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window != NULL)
	{
		aprilWindow->touchesCancelled_withEvent_(touches, event);
	}
}

// Handles the movement of a touch event. 
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	if (april::window != NULL)
	{
		aprilWindow->touchesMoved_withEvent_(touches, event);
	}
}

- (id)initWithFrame:(CGRect)frame
{
    self.displayLink = nil;
	if ((self = [super initWithFrame:frame]))
	{
		app_started = 0;
		frameInterval = 1;
		displayLinkAttached = false;
		// Get the layer
		CAEAGLLayer* eaglLayer = (CAEAGLLayer*) self.layer;
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
#ifdef _OPENGLES1
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#else
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#endif
		if (!context || ![EAGLContext setCurrentContext:context])
		{
			[self release];
			return nil;
		}
		
		// fake textbox
		CGRect textFrame = frame;
		textFrame.origin.x += frame.size.width;
		textField = [[UITextField alloc] initWithFrame:textFrame];
		textField.delegate = self;
		char longText[1025];
		memset(longText, ' ', 1024);
		longText[1024] = 0; // need to create large string because iPhone6 can move the caret manually and if it reaches the left end, backspace doesn't work anymore
		textField.text = [NSString stringWithUTF8String:longText]; // something to be able to catch text edit
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
		self.sensorManager = [[CMMotionManager alloc] init];
		// 60 FPS
		self.sensorManager.deviceMotionUpdateInterval = 0.016666667f;
		self.sensorManager.accelerometerUpdateInterval = 0.016666667f;
		self.sensorManager.gyroUpdateInterval = 0.016666667f;
		// create ios window object
		april::init(april::RenderSystemType::Default, april::WindowType::Default);
		april::createRenderSystem();
		april::createWindow(0, 0, true, "iOS Window");
		april::rendersys->update(0.0f);
	}
	return self;
}

- (BOOL)textFieldShouldReturn:(UITextField*)aTextField
{
	[textField resignFirstResponder];
	if (april::window != NULL)
	{
		aprilWindow->queueKeyInput(april::KeyEvent::Type::Down, april::Key::Return, 0);
		aprilWindow->queueKeyInput(april::KeyEvent::Type::Up, april::Key::Return, 0);
	}
	return YES;
}

// we'll also use this objc class for getting notifications
// on virtual keyboard's appearance and disappearance

- (void)keyboardWasShown:(NSNotification*)notification
{
	if (april::window != NULL)
	{
		NSDictionary* info = [notification userInfo];
		CGSize kbSize = [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;
		CGSize screenSize = getScreenBounds().size;
		// some iOS versions use portrait for default, so in some cases height is width 
		CGFloat kbHeight = hmin(kbSize.height, kbSize.width);
		CGFloat screenHeight = hmin(screenSize.height, screenSize.width);
		aprilWindow->keyboardWasShown(kbHeight / screenHeight);
	}
}

- (void)keyboardWasHidden:(id)sender
{
	if (april::window != NULL)
	{
		aprilWindow->keyboardWasHidden();
	}
}

-(void)deviceOrientationDidChange:(id)sender
{
}

- (void)drawView:(CADisplayLink*) link
{
    if (!self.displayLink)
    {
#ifdef _DEBUG
        hlog::write(april::logTag, "Called drawView while in background!");
#endif
        return;
    }
    [EAGLContext setCurrentContext:context];
	if (april::application != NULL)
	{
		[self updateSensors];
		april::application->update();
		if (april::application->getState() != april::Application::State::Running)
		{
			april::application->finish();
		}
	}
	if (frameInterval != link.frameInterval)
	{
		[link setFrameInterval:frameInterval];
	}
}

-(void)updateSensors
{
	april::MotionDelegate* motionDelegate = april::window->getMotionDelegate();
	bool gravity = false;
	bool linearAccelerometer = false;
	bool rotation = false;
	bool gyroscope = false;
	if (motionDelegate != NULL)
	{
		gravity = motionDelegate->isGravityEnabled();
		linearAccelerometer = motionDelegate->isLinearAccelerometerEnabled();
		rotation = motionDelegate->isRotationEnabled();
		gyroscope = motionDelegate->isGyroscopeEnabled();
	}
	if (gravity || rotation)
	{
		if (self.sensorManager.isDeviceMotionAvailable)
		{
			if (!self.sensorManager.isDeviceMotionActive)
			{
				[self.sensorManager startDeviceMotionUpdates];
			}
			if (gravity)
			{
				CMAcceleration motionVector = self.sensorManager.deviceMotion.gravity;
				april::window->queueMotionInput(april::MotionEvent::Type::Gravity, gvec3(motionVector.x * GRAVITY, motionVector.y * GRAVITY, motionVector.z * GRAVITY));
			}
			if (rotation)
			{
				CMRotationRate motionVector = self.sensorManager.deviceMotion.rotationRate;
				april::window->queueMotionInput(april::MotionEvent::Type::Rotation, gvec3(motionVector.x, motionVector.y, motionVector.z));
			}
		}
	}
	else if (self.sensorManager != NULL && self.sensorManager.isDeviceMotionActive)
	{
		[self.sensorManager stopDeviceMotionUpdates];
	}
	if (linearAccelerometer)
	{
		if (self.sensorManager.isAccelerometerAvailable)
		{
			if (!self.sensorManager.isAccelerometerActive)
			{
				[self.sensorManager startAccelerometerUpdates];
			}
			CMAcceleration motionVector = self.sensorManager.accelerometerData.acceleration;
			april::window->queueMotionInput(april::MotionEvent::Type::LinearAccelerometer, gvec3(motionVector.x, motionVector.y, motionVector.z));
		}
	}
	else if (self.sensorManager != NULL && self.sensorManager.isAccelerometerActive)
	{
		[self.sensorManager stopAccelerometerUpdates];
	}
	if (gyroscope)
	{
		if (self.sensorManager.isGyroAvailable)
		{
			if (!self.sensorManager.isGyroActive)
			{
				[self.sensorManager startGyroUpdates];
			}
			CMRotationRate motionVector = self.sensorManager.gyroData.rotationRate;
			april::window->queueMotionInput(april::MotionEvent::Type::Gyroscope, gvec3(motionVector.x, motionVector.y, motionVector.z));
		}
	}
	else if (self.sensorManager != NULL && self.sensorManager.isGyroActive)
	{
		[self.sensorManager stopGyroUpdates];
	}
}

- (void)_paintRect:(GLfloat[])vertices
{
}

- (void)swapBuffers
{
	if (self.displayLink == nil)
	{
#ifdef _DEBUG
		hlog::write(april::logTag, "Warning: OpenGL swapBuffers while app in background, ignoring!");
#endif
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
	NSString* depth = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"april_depth_buffer"];
	if (depth != nil)
	{
		glGenRenderbuffersOES(1, &depthRenderbuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
	}
	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		hlog::writef(april::logTag, "failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
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
	if (depthRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &depthRenderbuffer);
		depthRenderbuffer = 0;
	}
}

- (void)startAnimation
{
	if (!app_started)
	{
		return;
	}
	if (self.displayLink == nil)
	{
		self.displayLink = [[CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)] retain];
	}
	if (!displayLinkAttached)
	{
		displayLinkAttached = true;
		[self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
		[self.displayLink setFrameInterval:frameInterval];
	}
}

- (void)stopAnimation
{
	if (displayLinkAttached)
	{
		displayLinkAttached = false;
		[self.displayLink removeFromRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
	}
}

- (void)dealloc
{
	[self stopAnimation];
	if (self.displayLink != nil)
	{
		[self.displayLink invalidate];
		[self.displayLink release];
		self.displayLink = nil;
	}
	if ([EAGLContext currentContext] == context)
	{
		[EAGLContext setCurrentContext:nil];
	}
	[context release];
	[super dealloc];
}

- (void)showVirtualKeyboard
{
	[textField becomeFirstResponder];
}

- (BOOL)isVirtualKeyboardVisible
{
	return [textField isFirstResponder];
}

- (void)hideVirtualKeyboard
{
	[textField endEditing:YES];
}

- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string   // return NO to not change text
{
	if (april::window == NULL)
	{
		return NO;
	}
	hstr str = chstr([string UTF8String]);
	if (range.length == 1 && str.size() == 0)
	{
		aprilWindow->injectChar(0); // backspace indicator
	}
	else if (str.size() == 0)
	{
		return NO;
	}
	unichar chars[256];
	[string getCharacters:chars];
	int size = (int)[string length];
	for_iter (i, 0, size)
	{
		aprilWindow->injectChar(chars[i]);
	}
	return NO;
}

-(void)applicationDidBecomeActive:(UIApplication*)app
{
	if (april::window != NULL)
	{
		aprilWindow->applicationDidBecomeActive();
	}
}

-(void)applicationWillResignActive:(UIApplication*)app
{
	if (april::window != NULL)
	{
		aprilWindow->applicationWillResignActive();
	}
}

- (void)setUpdateInterval:(int)interval
{
	frameInterval = hmax(1, interval);
}

@end
