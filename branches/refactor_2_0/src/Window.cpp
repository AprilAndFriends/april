/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#include "iOSWindow.h"
#endif
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef _ANDROID
#include <jni.h>
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
	#import <Foundation/NSString.h>
	#import <AppKit/NSScreen.h>
	#import <AppKit/NSPanel.h>

	#import <AppKit/NSApplication.h>
	#import <AppKit/NSWindow.h>
	#import <AppKit/NSEvent.h>
	#import <AppKit/NSCursor.h>
#endif

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

#ifdef _ANDROID
namespace april
{
	extern void* javaVM;
	extern jobject aprilActivity;
	extern gvec2 androidResolution;
}
#endif

namespace april
{
	Window* window = NULL;
	
	Window::Window() : fullscreen(true), focused(true), running(true), cursorVisible(false)
	{
		april::window = this;
		this->updateCallback = NULL;
		this->mouseDownCallback = NULL;
		this->mouseUpCallback = NULL;
		this->mouseMoveCallback = NULL;
		this->keyDownCallback = NULL;
		this->keyUpCallback = NULL;
		this->charCallback = NULL;
		this->quitCallback = NULL;
		this->focusChangeCallback = NULL;
		this->touchscreenEnabledCallback = NULL;
		this->touchCallback = NULL;
		this->deviceOrientationCallback = NULL;
		this->virtualKeyboardCallback = NULL;
		this->handleUrlCallback = NULL;
		this->lowMemoryCallback = NULL;
	}
	
	Window::~Window()
	{
	}
	
	gvec2 Window::getSize()
	{
		return gvec2((float)this->getWidth(), (float)this->getHeight());
	}
	
	float Window::getAspectRatio()
	{
		return ((float)this->getWidth() / this->getHeight());
	}
	
	bool Window::isCursorInside()
	{
		return grect(0.0f, 0.0f, this->getSize()).isPointInside(this->getCursorPosition());
	}
	
	void Window::setMouseCallbacks(void (*mouseDownCallback)(float, float, int),
								   void (*mouseUpCallback)(float, float, int),
								   void (*mouseMoveCallback)(float, float))
	{
		this->mouseDownCallback = mouseDownCallback;
		this->mouseUpCallback = mouseUpCallback;
		this->mouseMoveCallback = mouseMoveCallback;
	}
	
	void Window::setKeyboardCallbacks(void (*keyDownCallback)(unsigned int),
									  void (*keyUpCallback)(unsigned int),
									  void (*charCallback)(unsigned int))
	{
		this->keyDownCallback = keyDownCallback;
		this->keyUpCallback = keyUpCallback;
		this->charCallback = charCallback;
	}
	
	bool Window::performUpdate(float k)
	{
		// returning true: continue execution
		// returning false: abort execution
		if (this->updateCallback != NULL)
		{
			return (*this->updateCallback)(k);
		}
		return true;
	}
	
	void Window::handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode)
	{
		this->_handleKeyOnlyEvent(type, keyCode);
		if (type == AKEYEVT_DOWN)
		{
			this->_handleCharOnlyEvent(charCode);
		}
	}
	
	void Window::_handleKeyOnlyEvent(KeyEventType type, KeySym keyCode)
	{
		if (keyCode == AK_UNKNOWN)
		{
			april::log("key event on unknown key");
			keyCode = AK_NONE;
		}
		switch (type)
		{
		case AKEYEVT_DOWN:
			if (this->keyDownCallback != NULL && keyCode != AK_NONE)
			{
				(*this->keyDownCallback)(keyCode);
			}
			break;
		case AKEYEVT_UP:
			if (this->keyUpCallback != NULL && keyCode != AK_NONE)
			{
				(*this->keyUpCallback)(keyCode);
			}
			break;
		default:
			break;
		}
	}
	
	void Window::_handleCharOnlyEvent(unsigned int charCode)
	{
		if (charCode > 0 && this->charCallback != NULL && charCode != 127) // special hack, backspace induces a character in some implementations
		{
			(*this->charCallback)(charCode);
		}
	}
	
	void Window::handleMouseEvent(MouseEventType type, gvec2 position, MouseButton button)
	{
		switch (type)
		{
		case AMOUSEEVT_DOWN:
			if (this->mouseDownCallback != NULL)
			{
				(*this->mouseDownCallback)(position.x, position.y, button);
			}
			break;
		case AMOUSEEVT_UP:
			if (this->mouseUpCallback != NULL)
			{
				(*this->mouseUpCallback)(position.x, position.y, button);
			}
			break;
		case AMOUSEEVT_MOVE:
			if (this->mouseMoveCallback != NULL)
			{
				(*this->mouseMoveCallback)(position.x, position.y);
			}
			break;
		}
	}
	
	void Window::handleTouchscreenEnabledEvent(bool enabled)
	{
		if (this->touchscreenEnabledCallback != NULL && this->isTouchEnabled() != enabled)
		{
			this->setTouchEnabled(enabled);
			(*this->touchscreenEnabledCallback)(enabled);
		}
	}
	
	void Window::handleTouchEvent(harray<gvec2>& touches)
	{
		if (this->touchCallback != NULL)
		{
			(*this->touchCallback)(touches);
		}
	}

	bool Window::handleQuitRequest(bool canReject)
	{
		// returns whether or not the windowing system is permitted to close the window
		if (this->quitCallback != NULL)
		{
			return (*this->quitCallback)(canReject);
		}
		return true;
	}
	
	void Window::handleFocusChangeEvent(bool focused)
	{
		this->focused = focused;
		if (this->focusChangeCallback != NULL)
		{
			(*this->focusChangeCallback)(focused);
		}
	}
	
	bool Window::handleUrl(chstr url)
	{
		if (this->handleUrlCallback != NULL)
		{
			return (*this->handleUrlCallback)(url);
		}
		return false;
	}
	
	void Window::handleLowMemoryWarning()
	{
		if (this->lowMemoryCallback != NULL)
		{
			(*this->lowMemoryCallback)();
		}
	}
	
	void Window::_platformCursorVisibilityUpdate(bool visible)
	{
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		NSWindow* window = [[NSApplication sharedApplication] keyWindow];
		bool shouldShow;
		
		if (!visible)
		{
			//NSPoint 	mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			//[window frame]
			NSPoint mouseLoc;
			id hideInsideView; // either NSView or NSWindow; both implement "frame" method
			if ([window contentView])
			{
				hideInsideView = [window contentView];
				mouseLoc = [window convertScreenToBase:[NSEvent mouseLocation]];
			}
			else
			{
				hideInsideView = window;
				mouseLoc = [NSEvent mouseLocation];
			}
			
			if (hideInsideView)
			{
				shouldShow = !NSPointInRect(mouseLoc, [hideInsideView frame]);
			}
			else // no view? let's presume we are in fullscreen where we should blindly honor the requests from the game
			{
				shouldShow = false;
			}
		}
		else
		{			
			shouldShow = true;
		}
		
		if (!shouldShow && CGCursorIsVisible())
		{
			CGDisplayHideCursor(kCGDirectMainDisplay);
		}
		else if (shouldShow && !CGCursorIsVisible())
		{
			CGDisplayShowCursor(kCGDirectMainDisplay);
		}
#endif
	}
	
}
