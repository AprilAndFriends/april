/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <UIKit/UIKit.h>
#import <QuartzCore/CAEAGLLayer.h>
#import <hltypes/exception.h>
#include <sys/sysctl.h>
#import "AprilViewController.h"
#import "EAGLView.h"
#import "iOS_Window.h"
#import "RenderSystem.h"
#import "ApriliOSAppDelegate.h"
#include "april.h"

static ApriliOSAppDelegate* appDelegate;
static UIWindow* uiwindow = NULL;
EAGLView* glview = NULL;
static AprilViewController* viewcontroller = NULL;

extern bool g_wnd_rotating;

namespace april
{
	InputEvent::InputEvent()
	{
	
	}
	
	InputEvent::~InputEvent()
	{
	
	}

	InputEvent::InputEvent(Window* window)
	{
		this->window = window;
	}

	class MouseInputEvent : public InputEvent
	{
	public:
		MouseInputEvent(Window* window, Window::MouseEventType type, gvec2 position, Window::MouseButton button) : InputEvent(window)
		{
			this->event = type;
			this->position = position;
			this->button = button;
		}
		
		void execute()
		{
			this->window->handleMouseEvent(this->type, this->position.x, this->position.y, this->button);
		}
		
	protected:
		Window::MouseEventType type;
		gvec2 position;
		Window::MouseButton button;
		
	};
	
	class TouchInputEvent : public InputEvent
	{
	public:
		TouchInputEvent(Window* window, harray<gvec2>& touches) : InputEvent(window)
		{
			this->touches = touches;
		}
		
		void execute()
		{
			this->window->handleTouchEvent(this->touches);
		}
		
	protected:
		harray<hstr> touches;
		
	};
	
	
    iOSWindow::iOSWindow() : Window()
    {
		this->name = APRIL_WS_IOS;
		this->keyboardRequest = 0;
		this->retainLoadingOverlay = false;
		this->inputEventsMutex = false;
		this->multiTouchActive = false;
		this->firstFrameDrawn = false; // show window after drawing first frame
	}
	
	iOSWindow::~iOSWindow()
	{
		destroy();
	}
	
    bool iOSWindow::create(int width, int height, bool fullscreen, chstr title)
    {
		if (!Window::create(width, height, fullscreen, title))
		{
			return false;
		}
		this->keyboardRequest = 0;
		this->retainLoadingOverlay = false;
		this->focused = true;
		this->inputEventsMutex = false;
		this->multiTouchActive = false;
		appDelegate = ((ApriliOSAppDelegate*)[[UIApplication sharedApplication] delegate]);
		viewcontroller = [appDelegate viewController];
		uiwindow = [appDelegate uiwindow];
		[UIApplication sharedApplication].statusBarHidden = fullscreen ? YES : NO;		
		this->fullscreen = true; // iOS apps are always fullscreen
		this->firstFrameDrawn = false; // show window after drawing first frame
		this->running = true;
		return true;
    }
	
    void iOSWindow::enterMainLoop()
    {
        NSLog(@"Fatal error: Using enterMainLoop on iOS!");
        exit(-1);
    }
	
	bool iOSWindow::updateOneFrame()
	{
		// call input events
		InputEvent* e;
		while ((e = this->popInputEvent()) != 0)
		{
			e->execute();
			delete e;
		}	
		if (this->keyboardRequest != 0 && this->touches.size() == 0) // only process keyboard when there is no interaction with the screen
		{
			bool visible = this->isVirtualKeyboardVisible();
			if (visible && this->keyboardRequest == -1)
			{
				[glview terminateKeyboardHandling];
			}
			else if (!visible && this->keyboardRequest == 1)
			{
				[glview beginKeyboardHandling];
			}
			this->keyboardRequest = 0;
		}
		
		float k = this->timer.diff(true);
		return this->performUpdate(k);	
	}

    void iOSWindow::terminateMainLoop()
	{
        NSLog(@"Fatal error: Using terminateMainLoop on iOS!");
        exit(-2);
    }
	
	void iOSWindow::destroyWindow()
	{
		// just stopping the animation on iOS
		[glview stopAnimation];
	}
	
    void iOSWindow::setCursorVisible(bool visible)
    {
        // no effect on iOS
    }
	
	void iOSWindow::addInputEvent(InputEvent* event)
	{
		// TODO - use a real mutex, this is unsafe
		while (this->inputEventsMutex); // wait it out
		this->inputEventsMutex = true;
		this->inputEvents += event;
		this->inputEventsMutex = false;
	}

	InputEvent* iOSWindow::popInputEvent()
	{
		// TODO - use a real mutex, this is unsafe
		while (this->inputEventsMutex); // wait it out
		if (this->inputEvents.size() == 0)
		{
			return NULL;
		}
		this->inputEventsMutex = true;
		InputEvent* e = this->inputEvents.pop_front();
		this->inputEventsMutex = false;
		return e;
	}

    bool iOSWindow::isCursorVisible()
    {
        return false; // iOS never shows system cursor
    }
	
    int iOSWindow::getWidth()
    {
		// TODO dont swap width and height in case display is in portrait mode
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		CAEAGLLayer *caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)])
		{
			return uiwindow.bounds.size.height * caeagllayer.contentsScale;
		}
#endif
        return uiwindow.bounds.size.height;
    }
	
    int iOSWindow::getHeight()
    {
		// TODO dont swap width and height in case display is in portrait mode
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		CAEAGLLayer *caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)])
		{
			return uiwindow.bounds.size.width * caeagllayer.contentsScale;
		}
#endif
        return uiwindow.bounds.size.width;
    }

    void iOSWindow::setTitle(chstr value)
    {
        // no effect on iOS
    }
	
    void iOSWindow::presentFrame()
    {
		if (this->firstFrameDrawn)
		{
			[glview swapBuffers];
		}
		else
		{
			this->checkEvents();
			if (!this->retainLoadingOverlay)
			{
				[viewcontroller removeImageView];
			}
			this->firstFrameDrawn = true;
		}
    }

	void* iOSWindow::getIdFromBackend()
	{
		return viewcontroller;
	}

	void iOSWindow::checkEvents()
	{
		SInt32 result;
		do
		{
			result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
		} while (result == kCFRunLoopRunHandledSource);
	}
	
	void iOSWindow::callTouchCallback()
	{
		if (this->touchCallback == NULL)
		{
			return;
		}
		harray<gvec2> coordinates;
		gvec2 position;
		CGPoint point;
		float scale = this->_getTouchScale();
		
		foreach (UITouch*, it, this->touches)
		{
			point = [*it locationInView:glview];
			position.x = point.x * scale;
			position.y = point.y * scale;
			coordinates += position;
		}
		this->inputEvents += new TouchInputEvent(this, coordinates);
	}
	
	bool iOSWindow::isRotating()
	{
		return g_wnd_rotating;
	}
	
	hstr iOSWindow::getParam(chstr param)
	{
		if (param == "retain_loading_overlay")
		{
			return this->retainLoadingOverlay ? "1" : "0";
		}
		return "";
	}
	
	void iOSWindow::setParam(chstr param, chstr value)
	{
		if (param == "retain_loading_overlay")
		{
			bool prev = this->retainLoadingOverlay;
			this->retainLoadingOverlay = (value != "0");
			if (!this->retainLoadingOverlay && prev && this->firstFrameDrawn)
			{
				[viewcontroller removeImageView];
			}
		}
	}

	float iOSWindow::_getTouchScale()
	{
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		static float scale = -1;
		if (scale == -1)
		{
			CAEAGLLayer* caeagllayer = (CAEAGLLayer*)[glview layer];
			if ([caeagllayer respondsToSelector:@selector(contentsScale)])
			{
				scale = [caeagllayer contentsScale];
			}
			else
			{
				scale = 1; // prior to ios 3.2
			}
		}
		return scale;
#else
		return 1;
#endif
	}

	harray<UITouch*> iOSWindow::_convertTouchesToCoordinates(void* nssetTouches)
	{
		float scale = this->_getTouchScale();
		// return value stored in cursorX and cursorY
		harray<UITouch*> coordinates;
		NSSet* touches = (NSSet*)nssetTouches;
		UITouch* touch;
		int len = [touches count];

		if (len == 1)
		{
			touch = touches.anyObject;
			CGPoint location = [touch locationInView:glview];
			//For "primary" landscape orientation, this is how we calc it
			this->cursorPosition.x = location.x * scale;
			this->cursorPosition.y = location.y * scale;
			coordinates += touch;
		}
		else
		{
			for (touch in touches)
			{
				coordinates += touch;
			}
		}
		
		return coordinates;
	}
	
	void iOSWindow::touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		harray<UITouch*> touches = this->_convertTouchesToCoordinates(nssetTouches);
		
		this->touches += touches;
		if (this->touches.size() > 1)
		{
			if (!this->multiTouchActive && this->touches.size() == 1)
			{
				// cancel (notify the app) the previously called mousedown event so we can begin the multi touch event properly
				this->addInputEvent(new MouseInputEvent(this, AMOUSEEVT_UP, -10000, -10000, AMOUSEBTN_LEFT));
			}
			this->multiTouchActive = true;
		}
		else
		{
			this->addInputEvent(new MouseInputEvent(this, AMOUSEEVT_DOWN, this->cursorPosition, AMOUSEBTN_LEFT));
		}
		this->callTouchCallback();
	}

	void iOSWindow::touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		harray<UITouch*> touches = this->_convertTouchesToCoordinates(nssetTouches);
		int num_touches = this->touches.size();
		this->touches /= touches;
		
		if (this->multiTouchActive)
		{
			if (num_touches == touches.size())
			{
				this->multiTouchActive = false;
			}
		}
		else
		{
			this->addInputEvent(new MouseInputEvent(this, AMOUSEEVT_UP, this->cursorPosition, AMOUSEBTN_LEFT));
		}
		this->callTouchCallback();
	}
	
	
	void iOSWindow::touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		// FIXME needs to cancel touches, not treat them as "release"
		this->touchesEnded_withEvent_(nssetTouches, uieventEvent);
	}
	
	void iOSWindow::touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		this->_convertTouchesToCoordinates(nssetTouches);
		this->addInputEvent(new MouseInputEvent(this, AMOUSEEVT_MOVE, this->cursorPosition, AMOUSEBTN_NONE));
		this->callTouchCallback();
	}
	
	bool iOSWindow::isVirtualKeyboardVisible()
	{
		return [glview isKeyboardActive];
	}
	
	void iOSWindow::beginKeyboardHandling()
	{
		this->keyboardRequest = 1;
	}
	
	void iOSWindow::terminateKeyboardHandling()
	{
		this->keyboardRequest = -1;
	}
	
	void iOSWindow::injectiOSChar(unsigned int inputChar)
	{
		if (inputChar == 0)
		{
			// deploy backspace
			this->handleKeyEvent(AKEYEVT_DOWN, AK_BACK, 8);
			this->handleKeyEvent(AKEYEVT_UP, AK_BACK, 8);
		}
		if (inputChar >= 32)
		{
			// deploy keypress
			april::KeySym keycode = AK_NONE; // TODO - FIXME incorrect, might cause a nasty bug. 
											 // however, writing a translation table atm 
											 // isn't the priority.
		
			this->handleKeyEvent(AKEYEVT_DOWN, keycode, inputChar);
			this->handleKeyEvent(AKEYEVT_UP, keycode, inputChar);
		}
	}
	
	void iOSWindow::keyboardWasShown()
	{
		if (this->virtualKeyboardCallback != NULL)
		{
			(*this->virtualKeyboardCallback)(true);
		}
	}
	void iOSWindow::keyboardWasHidden()
	{
		if (this->virtualKeyboardCallback != NULL)
		{
			(*this->virtualKeyboardCallback)(false);
		}
	}
	
	void iOSWindow::setDeviceOrientationCallback(void (*do_callback)(DeviceOrientation))
	{
		if (do_callback != NULL)
		{
			[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
		}
		else
		{
			[[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
		}
		// update mDeviceOrientationCallback
		Window::setDeviceOrientationCallback(do_callback);
	}
	
	//////////////
	void iOSWindow::handleDisplayAndUpdate()
	{
		this->updateOneFrame();
		april::rendersys->presentFrame();
	}
	
	void iOSWindow::deviceOrientationDidChange()
	{
		if (this->deviceOrientationCallback != NULL)
		{
			DeviceOrientation newOrientation;
			switch ([[UIDevice currentDevice] orientation])
			{
			case UIDeviceOrientationUnknown:
				newOrientation = ADEVICEORIENTATION_NONE;
			case UIDeviceOrientationPortrait:
				newOrientation = ADEVICEORIENTATION_PORTRAIT;
			case UIDeviceOrientationPortraitUpsideDown:
				newOrientation = ADEVICEORIENTATION_PORTRAIT_UPSIDEDOWN;
			case UIDeviceOrientationLandscapeLeft:
				newOrientation = ADEVICEORIENTATION_LANDSCAPE_LEFT;
			case UIDeviceOrientationLandscapeRight:
				newOrientation = ADEVICEORIENTATION_LANDSCAPE_RIGHT;
			case UIDeviceOrientationFaceUp:
				newOrientation = ADEVICEORIENTATION_FACE_UP;
			case UIDeviceOrientationFaceDown:
				newOrientation = ADEVICEORIENTATION_FACE_DOWN;
			// not adding default, so we get a warning in case
			// a new orientation is added to backing API
			}
			newOrientation = ADEVICEORIENTATION_NONE;
			(*this->deviceOrientationCallback)(newOrientation);
		}
	}
	void iOSWindow::applicationWillResignActive()
	{
		if (!this->firstFrameDrawn)
		{
			log("April iOS Window: received app suspend request before first frame was drawn, quitting app.");
			this->destroy();
			exit(0);
		}
		if (this->focused)
		{
			this->focused = false;
			if (this->focusCallback != NULL)
			{
				(*this->focusCallback)(false);
			}
			[glview stopAnimation];
		}
	}
	
	void iOSWindow::applicationDidBecomeActive()
	{
		if (!this->focused)
		{
			this->focused = true;
			[glview startAnimation];
			if (this->focusCallback != NULL)
			{
				(*this->focusCallback)(true);
			}
		}
	}

}
