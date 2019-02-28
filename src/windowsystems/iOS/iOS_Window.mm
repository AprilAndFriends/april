/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _IOS_WINDOW
#import <UIKit/UIKit.h>
#import <QuartzCore/CAEAGLLayer.h>

#include <hltypes/hexception.h>
#include <hltypes/hlog.h>

#import "AprilViewController.h"
#import "EAGLView.h"
#import "ApriliOSAppDelegate.h"

#include "Application.h"
#include "april.h"
#include "EventDelegate.h"
#include "iOS_Window.h"
#include "RenderSystem.h"

static ApriliOSAppDelegate* appDelegate;
static UIWindow* uiwindow = NULL;
EAGLView* glview = NULL;
static AprilViewController* viewcontroller = NULL;

namespace april
{
	// TODOx - convert to gvec2f so it can be included in the class
	static harray<UITouch*> g_touches;
	
	// TODOx - probably not needed anymore
	void updateCursorPosition(gvec2f touch)
	{
		float scale = ((iOS_Window*) window)->_getTouchScale();
		// return value stored in cursorX and cursorY		
		//For "primary" landscape orientation, this is how we calc it
		//hlog::errorf("OK", "%4.0f %4.0f", touch.x * scale, touch.y * scale);	// for debugging
		((iOS_Window*) window)->_setCursorPosition(touch.x * scale, touch.y * scale);
	}
	
	static harray<UITouch*> _convertTouchesToCoordinates(void* nssetTouches)
	{
		// return value stored in cursorX and cursorY
		harray<UITouch*> coordinates;
		NSSet* touches = (NSSet*)nssetTouches;
		UITouch* touch;
		for (touch in touches)
		{
			coordinates += touch;
		}
		return coordinates;
	}

	iOS_Window::iOS_Window() :
		Window()
	{
		this->name = april::WindowType::iOS.getName();
		this->width = 0;
		this->height = 0;
		this->exitFunction = NULL;
	}
	
	void iOS_Window::_systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		if (options.minimized)
		{
			options.minimized = false;
			hlog::warn(logTag, "Option 'minimized' is not supported on window system: " + this->name);
		}
		Window::_systemCreate(width, height, fullscreen, title, options);
		this->firstFrameDrawn = false; // show window after drawing first frame
		this->keyboardRequest = 0;
		this->retainLoadingOverlay = false;
		this->inputMode = InputMode::Touch;
		this->focused = true;
		this->inputEventsMutex = false;
		this->multiTouchActive = false;
		appDelegate = ((ApriliOSAppDelegate*)[[UIApplication sharedApplication] delegate]);
		viewcontroller = [appDelegate viewController];
		uiwindow = appDelegate.uiwnd;
		[UIApplication sharedApplication].statusBarHidden = fullscreen ? YES : NO;		
		this->fullscreen = true; // iOS apps are always fullscreen
		this->firstFrameDrawn = false; // show window after drawing first frame
		float scale = 1.0f;
		CAEAGLLayer* caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)])
		{
			scale = caeagllayer.contentsScale;
		}
		CGRect bounds = uiwindow.bounds;
		this->width = bounds.size.width * scale;
		this->height = bounds.size.height * scale;
	}
	
	iOS_Window::~iOS_Window()
	{
		this->destroy();
	}
	
	void iOS_Window::checkEvents()
	{
		bool visible = this->isVirtualKeyboardVisible();
		this->virtualKeyboardVisible = visible;
		if (this->keyboardRequest != 0 && g_touches.size() == 0) // only process keyboard when there is no interaction with the screen
		{
			if (visible && this->keyboardRequest == -1)
			{
				[glview hideVirtualKeyboard];
			}
			else if (!visible && this->keyboardRequest == 1)
			{
				[glview showVirtualKeyboard];
			}
			this->keyboardRequest = 0;
		}
		float scale = 1.0f;
		CAEAGLLayer* caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)])
		{
			scale = caeagllayer.contentsScale;
		}
		CGRect bounds = uiwindow.bounds;
		this->width = bounds.size.width * scale;
		this->height = bounds.size.height * scale;
		Window::checkEvents();
	}

	void iOS_Window::destroyWindow()
	{
		// just stopping the animation on iOS
		[glview stopAnimation];
	}
	
	void iOS_Window::setCursorVisible(bool visible)
	{
		// no effect on iOS
	}
	
	void iOS_Window::_setCursorPosition(float x, float y)
	{
		this->cursorPosition.set(x, y);
	}
	
	Cursor* iOS_Window::_createCursor(bool fromResource)
	{
		return NULL;
	}

	bool iOS_Window::isCursorVisible() const
	{
		return false; // iOS never shows system cursor
	}

	void iOS_Window::setTitle(chstr value)
	{
		// no effect on iOS
	}
	
	void* iOS_Window::getBackendId() const
	{
		return viewcontroller;
	}

	void iOS_Window::_presentFrame(bool systemEnabled)
	{
		Window::_presentFrame(systemEnabled);
		if (this->firstFrameDrawn)
		{
			[glview swapBuffers];
		}
		else
		{
			this->_processEvents();
			if (!this->retainLoadingOverlay)
			{
				[viewcontroller removeImageView:false];
			}
			this->firstFrameDrawn = true;
		}
	}

	void iOS_Window::_processEvents()
	{
		// TODOx - implement proper cursor update
		/*
		float scale = this->_getTouchScale();
		this->_setCursorPosition(touch.x * scale, touch.y * scale);
		*/
		Window::_processEvents();
	}
	
	hstr iOS_Window::getParam(chstr param)
	{
		if (param == "retain_loading_overlay")
		{
			return this->retainLoadingOverlay ? "1" : "0";
		}
		return Window::getParam(param);
	}
	
	void iOS_Window::setParam(chstr param, chstr value)
	{
		if (param == "retain_loading_overlay")
		{
			bool prev = this->retainLoadingOverlay;
			this->retainLoadingOverlay = (value == "1"); // TODO - should use true/false
			if (!this->retainLoadingOverlay && prev)// && this->firstFrameDrawn)
			{
				[viewcontroller removeImageView:(value == "0" ? false : true)];
			}
			return;
		}
		if (param == "exit_function")
		{
			unsigned long ptr = 0;
			sscanf(value.cStr(), "%lu", &ptr);
			this->exitFunction = (void (*)(int)) ptr;
			return;
		}
		if (param == "CADisplayLink::updateInterval" && glview != nil)
		{
			[glview setUpdateInterval:(int)value];
			return;
		}
		Window::setParam(param, value);
	}
	
	void iOS_Window::handleFocusChange(bool focused)
	{
		if (this->focused != focused) // this can happen on iOS
		{
			Window::handleFocusChange(focused);
		}
	}

	float iOS_Window::_getTouchScale() const
	{
		static float scale = -1.0f;
		if (scale < 0.0f)
		{
			CAEAGLLayer* caeagllayer = (CAEAGLLayer*)[glview layer];
			if ([caeagllayer respondsToSelector:@selector(contentsScale)])
			{
				scale = [caeagllayer contentsScale];
			}
			else
			{
				scale = 1.0f; // prior to ios 3.2
			}
		}
		return scale;
	}
	
	void iOS_Window::touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		harray<UITouch*> touches = _convertTouchesToCoordinates(nssetTouches);
		int previousSize = g_touches.size();
		g_touches += touches;
		CGPoint point;
		float scale = this->_getTouchScale();
		for_iter (i, 0, touches.size())
		{
			point = [touches[i] locationInView:glview];
			this->queueTouchInput(TouchEvent::Type::Down, previousSize + i, gvec2f(point.x * scale, point.y * scale));
		}
	}

	void iOS_Window::touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		harray<UITouch*> touches = _convertTouchesToCoordinates(nssetTouches);
		harray<int> indices;
		for_iter (i, 0, g_touches.size())
		{
			if (touches.has(g_touches[i]))
			{
				indices += i;
			}
		}
		g_touches /= touches;
		CGPoint point;
		float scale = this->_getTouchScale();
		for_iter (i, 0, touches.size())
		{
			point = [touches[i] locationInView:glview];
			this->queueTouchInput(TouchEvent::Type::Up, indices[i], gvec2f(point.x * scale, point.y * scale));
		}
	}
	
	void iOS_Window::touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		// TODOx - still needs to be implemented properly
		this->touchesEnded_withEvent_(nssetTouches, uieventEvent);
	}
	
	void iOS_Window::touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		harray<UITouch*> touches = _convertTouchesToCoordinates(nssetTouches);
		harray<int> indices;
		for_iter (i, 0, g_touches.size())
		{
			if (touches.has(g_touches[i]))
			{
				indices += i;
			}
		}
		CGPoint point;
		float scale = this->_getTouchScale();
		for_iter (i, 0, touches.size())
		{
			point = [touches[i] locationInView:glview];
			this->queueTouchInput(TouchEvent::Type::Move, indices[i], gvec2f(point.x * scale, point.y * scale));
		}
	}
	
	// TODOx - maybe this should be handled here and instead the Window superclass should keep handling this like in other implementations
	bool iOS_Window::isVirtualKeyboardVisible() const
	{
		return [glview isVirtualKeyboardVisible];
	}
	
	void iOS_Window::showVirtualKeyboard()
	{
		this->keyboardRequest = 1;
	}
	
	void iOS_Window::hideVirtualKeyboard()
	{
		this->keyboardRequest = -1;
	}
	
	void iOS_Window::injectChar(unsigned int inputChar)
	{
		if (inputChar == 0)
		{
			// deploy backspace
			this->queueKeyInput(KeyEvent::Type::Down, Key::Backspace, 8);
			this->queueKeyInput(KeyEvent::Type::Up, Key::Backspace, 8);
		}
		else if (inputChar >= 32)
		{
			this->queueKeyInput(KeyEvent::Type::Down, Key::None, inputChar);
			this->queueKeyInput(KeyEvent::Type::Up, Key::None, inputChar);
		}
	}
	
	void iOS_Window::keyboardWasShown(float kbSize)
	{
		if (this->systemDelegate != NULL)
		{
			this->handleVirtualKeyboardChange(true, kbSize);
		}
	}
	
	void iOS_Window::keyboardWasHidden()
	{
		if (this->systemDelegate != NULL)
		{
			this->handleVirtualKeyboardChange(false, 0.0f);
		}
	}
	
	void iOS_Window::applicationWillResignActive()
	{
		if (!this->firstFrameDrawn)
		{
			hlog::warn(logTag, "iOS Window: received app suspend request before first frame was drawn");
			// commenting this code, not relevant on iOS4.3+
			//hlog::write(logTag, "iOS Window: received app suspend request before first frame was drawn, quitting app.");
			//this->destroy();
			//exit(0);
		}
		// TODOx - is this "if" required?
		//if (this->focused)
		{
			april::application->suspend();
			this->queueFocusChange(false);
			[glview stopAnimation];
		}
	}
	
	void iOS_Window::applicationDidBecomeActive()
	{
		// TODOx - is this "if" required?
		//if (!this->focused)
		{
			april::application->resume();
			this->queueFocusChange(true);
			if (glview != NULL)
			{
				[glview startAnimation];
			}
		}
	}

	void iOS_Window::terminateMainLoop()
	{
		// TODOx - should be removed completely probably
		// apple doesn't approve apps exiting via exit() so we have to camoufluage it if we want to use it
		if (this->exitFunction)
		{
			this->exitFunction(0);
		}
		else
		{
			hlog::write(logTag, "april's iOS_Window doesn't have the 'exitFunction' property set, you need to set it if you want to manually exit the app. Be warned though, apple strongly discourages this behaviour.");
		}
		exit(0);
	}
}

CGRect getScreenBounds()
{
	CGRect frame = [[UIScreen mainScreen] bounds];
/*	if ([[[UIDevice currentDevice] systemVersion] compare:@"8.0" options:NSNumericSearch] != NSOrderedAscending)
	{
		// iOS8 has inverted logic for landscape, so counter it...
		float temp = frame.size.width;
		frame.size.width = frame.size.height;
		frame.size.height = temp;
	}
*/
	return frame;
}
#endif
