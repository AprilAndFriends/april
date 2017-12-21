/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <UIKit/UIKit.h>
#import <QuartzCore/CAEAGLLayer.h>
#import <hltypes/hexception.h>
#import <hltypes/hlog.h>

#import "AprilViewController.h"
#import "EAGLView.h"
#include "iOS_Window.h"
#import "RenderSystem.h"
#import "ApriliOSAppDelegate.h"

#include "EventDelegate.h"

#include "april.h"

static ApriliOSAppDelegate* appDelegate;
static UIWindow* uiwindow = NULL;
EAGLView* glview = NULL;
static AprilViewController* viewcontroller = NULL;

extern bool g_wnd_rotating;

namespace april
{
	// TODO - convert to gvec2 so it can be included in the class
	static harray<UITouch*> g_touches;
	
	void updateCursorPosition(gvec2 touch)
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

	class iOS_MouseInputEvent : public InputEvent
	{
	public:
		iOS_MouseInputEvent(Window* window, Window::MouseInputEvent::Type type, gvec2 position, april::Key button) : InputEvent(window)
		{
			this->type = type;
			this->position = position;
			this->button = button;
		}
		
		void execute()
		{
			if (this->type != Window::MouseInputEvent::Type::Cancel) updateCursorPosition(this->position);
			this->window->handleMouseEvent(this->type, this->position * ((iOS_Window*) window)->_getTouchScale(), this->button);
		}
		
	protected:
		Window::MouseInputEvent::Type type;
		gvec2 position;
		april::Key button;
		
	};
	
	class iOS_TouchInputEvent : public InputEvent
	{
	public:
		iOS_TouchInputEvent(Window* window, harray<gvec2>& touches) : InputEvent(window)
		{
			float scale = ((iOS_Window*)window)->_getTouchScale();
			foreach (gvec2, it, touches)
			{
				this->touches += (*it) * scale;
			}
		}
		
		void execute()
		{
			this->window->handleTouchEvent(this->touches);
		}
		
	protected:
		harray<gvec2> touches;
		
	};
	
	iOS_Window::iOS_Window() : Window()
	{
		this->name = april::WindowType::iOS.getName();
		this->exitFunction = NULL;
	}
	
	bool iOS_Window::_systemCreate(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::_systemCreate(w, h, fullscreen, title, options))
		{
			return false;
		}
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
		this->running = true;
		return true;
	}
	
	iOS_Window::~iOS_Window()
	{
		this->destroy();
	}
		
	bool iOS_Window::update(float timeDelta)
	{
		// call input events
		InputEvent* e;
		while ((e = this->popInputEvent()) != 0)
		{
			e->execute();
			delete e;
		}	
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
		return Window::update(timeDelta);
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
	
	void iOS_Window::addInputEvent(InputEvent* event)
	{
		// TODO - use a real mutex, this is unsafe
		while (this->inputEventsMutex); // wait it out
		this->inputEventsMutex = true;
		this->inputEvents += event;
		this->inputEventsMutex = false;
	}

	InputEvent* iOS_Window::popInputEvent()
	{
		// TODO - use a real mutex, this is unsafe
		while (this->inputEventsMutex); // wait it out
		if (this->inputEvents.size() == 0)
		{
			return NULL;
		}
		this->inputEventsMutex = true;
		InputEvent* e = this->inputEvents.removeFirst();
		this->inputEventsMutex = false;
		return e;
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
	
	int iOS_Window::getWidth() const
	{
		float scale = 1;
		CAEAGLLayer *caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)])
		{
			scale = caeagllayer.contentsScale;
		}
		CGRect bounds = uiwindow.bounds;
		if (isiOS8OrNewer() || UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation]))
		{
			return bounds.size.width * scale;
		}
		return bounds.size.height * scale;
	}

	int iOS_Window::getHeight() const
	{
		float scale = 1;
		CAEAGLLayer *caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)])
		{
			scale = caeagllayer.contentsScale;
		}
		CGRect bounds = uiwindow.bounds;
		if (isiOS8OrNewer() || UIInterfaceOrientationIsPortrait([[UIApplication sharedApplication] statusBarOrientation]))
		{
			return bounds.size.height * scale;
		}
		return bounds.size.width * scale;
	}

	void iOS_Window::setTitle(chstr value)
	{
		// no effect on iOS
	}
	
	void* iOS_Window::getBackendId() const
	{
		return viewcontroller;
	}

	void iOS_Window::_presentFrame()
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
				[viewcontroller removeImageView:false];
			}
			this->firstFrameDrawn = true;
		}
	}

	void iOS_Window::checkEvents()
	{
		// TODO - handling is done manually somewhere else, needs refactoring
		//Window::checkEvents();
	}
	
	void iOS_Window::callTouchCallback()
	{
		if (this->touchDelegate == NULL)
		{
			return;
		}
		harray<gvec2> coordinates;
		gvec2 position;
		CGPoint point;
		float scale = this->_getTouchScale();
		
		foreach (UITouch*, it, g_touches)
		{
			point = [*it locationInView:glview];
			position.x = point.x * scale;
			position.y = point.y * scale;
			coordinates += position;
		}
		this->inputEvents += new iOS_TouchInputEvent(this, coordinates);
	}
	
	bool iOS_Window::isRotating() const
	{
		return g_wnd_rotating;
	}
	
	hstr iOS_Window::getParam(chstr param)
	{
		if (param == "retain_loading_overlay")
		{
			return this->retainLoadingOverlay ? "1" : "0";
		}
		return "";
	}
	
	void iOS_Window::setParam(chstr param, chstr value)
	{
		if (param == "retain_loading_overlay")
		{
			bool prev = this->retainLoadingOverlay;
			this->retainLoadingOverlay = (value == "1");
			if (!this->retainLoadingOverlay && prev && this->firstFrameDrawn)
			{
				[viewcontroller removeImageView:(value == "0" ? false : true)];
			}
		}
		if (param == "exit_function")
		{
			unsigned long ptr;
			sscanf(value.cStr(), "%lu", &ptr);
			this->exitFunction = (void (*)(int)) ptr;
		}
		if (param == "CADisplayLink::updateInterval" && glview != nil)
		{
			[glview setUpdateInterval:(int)value];
		}
	}

	float iOS_Window::_getTouchScale() const
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
	
	void iOS_Window::touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		harray<UITouch*> touches = _convertTouchesToCoordinates(nssetTouches);
		
		int prev_len = g_touches.size();
		g_touches += touches;
		if (g_touches.size() > 1)
		{
			if (!this->multiTouchActive && prev_len == 1)
			{
				// cancel (notify the app) the previously called mousedown event so we can begin the multi touch event properly
				this->addInputEvent(new iOS_MouseInputEvent(this, Window::MouseInputEvent::Type::Cancel, gvec2(), april::Key::MouseL));
			}
			this->multiTouchActive = true;
		}
		else
		{
			CGPoint pt = [g_touches[0] locationInView:glview];
			this->addInputEvent(new iOS_MouseInputEvent(this, Window::MouseInputEvent::Type::Down, gvec2(pt.x, pt.y), april::Key::MouseL));
		}
		this->callTouchCallback();
	}

	void iOS_Window::touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		harray<UITouch*> touches = _convertTouchesToCoordinates(nssetTouches);
		int num_touches = g_touches.size();
		g_touches /= touches;
		
		if (this->multiTouchActive)
		{
			if (num_touches == touches.size())
			{
				this->multiTouchActive = false;
			}
		}
		else
		{
			CGPoint pt = [touches[0] locationInView:glview];
			this->addInputEvent(new iOS_MouseInputEvent(this, Window::MouseInputEvent::Type::Up, gvec2(pt.x, pt.y), april::Key::MouseL));
		}
		this->callTouchCallback();
	}
	
	void iOS_Window::touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		// FIXME needs to cancel touches, not treat them as "release"
		this->touchesEnded_withEvent_(nssetTouches, uieventEvent);
	}
	
	void iOS_Window::touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		if (!this->multiTouchActive)
		{
			UITouch* touch = [[(NSSet*) nssetTouches allObjects] objectAtIndex:0];
			CGPoint pt = [touch locationInView:glview];			
			this->addInputEvent(new iOS_MouseInputEvent(this, Window::MouseInputEvent::Type::Move, gvec2(pt.x, pt.y), april::Key::None));
		}
		this->callTouchCallback();
	}
	
	// TODOa - maybe this should be handle here and instead the Window superclass should keep handling this like in other implementations
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
	
	void iOS_Window::injectiOSChar(unsigned int inputChar)
	{
		if (inputChar == 0)
		{
			// deploy backspace
			this->handleKeyEvent(Window::KeyInputEvent::Type::Down, april::Key::Backspace, 8);
			this->handleKeyEvent(Window::KeyInputEvent::Type::Up, april::Key::Backspace, 8);
		}
		if (inputChar >= 32)
		{
			// deploy keypress
			april::Key keycode = april::Key::None; // TODO - FIXME incorrect, might cause a nasty bug.
											       // however, writing a translation table atm
											       // isn't the priority.
		
			this->handleKeyEvent(Window::KeyInputEvent::Type::Down, keycode, inputChar);
			this->handleKeyEvent(Window::KeyInputEvent::Type::Up, keycode, inputChar);
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
	
	void iOS_Window::handleDisplayAndUpdate()
	{
		bool result = this->updateOneFrame();
		if (april::rendersys != NULL)
		{
			april::rendersys->update();
		}
		if (this->updateDelegate != NULL)
		{
			this->updateDelegate->onPresentFrame();
		}
		if (!result)
		{
			this->terminateMainLoop();
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
		if (this->focused)
		{
			this->focused = false;
			if (this->systemDelegate != NULL)
			{
				this->systemDelegate->onWindowFocusChanged(false);
			}
			[glview stopAnimation];
		}
	}
	
	void iOS_Window::applicationDidBecomeActive()
	{
		if (!this->focused)
		{
			this->focused = true;
			if (g_touches.size() > 0) // in some situations, on older iOS versions (happend on iOS4), touchesEnded will not be called, causing problems, so this counters it.
			{
				g_touches.clear();
				multiTouchActive = false;
			}
			if (glview != NULL)
			{
				[glview startAnimation];
			}
			if (this->systemDelegate != NULL)
			{
				this->systemDelegate->onWindowFocusChanged(true);
			}
		}
	}

	void iOS_Window::terminateMainLoop()
	{
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

bool isiOS8OrNewer()
{
	static int value = -1;
	if (value == -1)
	{
		value = [[[UIDevice currentDevice] systemVersion] compare:@"8.0" options:NSNumericSearch] != NSOrderedAscending;
	}
	return value == 1;
}
