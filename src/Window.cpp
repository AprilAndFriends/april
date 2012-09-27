/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.32
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	// TODO - refactor
	void (*Window::msLaunchCallback)(void*) = NULL;
	void Window::handleLaunchCallback(void* args)
	{
		if (msLaunchCallback != NULL)
		{
			(*msLaunchCallback)(args);
		}
	}
	//////////////////

	Window* window = NULL;
	
	Window::Window() : created(false), fullscreen(true), focused(true), running(true), cursorVisible(false)
	{
		april::window = this;
		this->name = "Generic";
		this->updateCallback = NULL;
		this->mouseDownCallback = NULL;
		this->mouseUpCallback = NULL;
		this->mouseMoveCallback = NULL;
		this->mouseScrollCallback = NULL;
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
		this->mouseMoveCallback_DEPRECATED = NULL;
		this->mouseUpCallback_DEPRECATED = NULL;
		this->mouseDownCallback_DEPRECATED = NULL;
	}
	
	Window::~Window()
	{
		this->destroy();
	}

	bool Window::create(int w, int h, bool fullscreen, chstr title)
	{
		if (!this->created)
		{
			april::log(hsprintf("creating window '%s' (%d, %d), '%s' fullscreen : %s", this->name.c_str(), w, h, title.c_str(), fullscreen ? "yes" : "no"));
			this->fullscreen = fullscreen;
			this->title = title;
			this->created = true;
			return true;
		}
		return false;
	}
	
	bool Window::destroy()
	{
		if (this->created)
		{
			april::log(hsprintf("destroying window '%s'", this->name.c_str()));
			this->created = false;
			return true;
		}
		return false;
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
	
	void Window::setMouseCallbacks(void (*mouseDownCallback)(int),
								   void (*mouseUpCallback)(int),
								   void (*mouseMoveCallback)(),
								   void (*mouseScrollCallback)(float, float))
	{
		this->mouseDownCallback = mouseDownCallback;
		this->mouseUpCallback = mouseUpCallback;
		this->mouseMoveCallback = mouseMoveCallback;
		this->mouseScrollCallback = mouseScrollCallback;
		this->mouseDownCallback_DEPRECATED = NULL;
		this->mouseUpCallback_DEPRECATED = NULL;
		this->mouseMoveCallback_DEPRECATED = NULL;
	}
	
	void Window::setKeyboardCallbacks(void (*keyDownCallback)(unsigned int),
									  void (*keyUpCallback)(unsigned int),
									  void (*charCallback)(unsigned int))
	{
		this->keyDownCallback = keyDownCallback;
		this->keyUpCallback = keyUpCallback;
		this->charCallback = charCallback;
	}
	
	void Window::enterMainLoop()
	{
		this->running = true;
		while (this->running)
		{
			if (!this->updateOneFrame())
			{
				this->running = false;
			}
		}
	}

	bool Window::updateOneFrame()
	{
		return this->running;
	}
	
	void Window::terminateMainLoop()
	{
		this->running = false;
	}
	
	bool Window::performUpdate(float k)
	{
		// returning true: continue execution
		// returning false: abort execution
		if (this->updateCallback != NULL)
		{
			return (*this->updateCallback)(k);
		}
		rendersys->clear();
		return true;
	}
	
	void Window::handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode)
	{
		this->handleKeyOnlyEvent(type, keyCode);
		if (type == AKEYEVT_DOWN)
		{
			this->handleCharOnlyEvent(charCode);
		}
	}
	
	void Window::handleKeyOnlyEvent(KeyEventType type, KeySym keyCode)
	{
		if (keyCode == AK_UNKNOWN)
		{	
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
	
	void Window::handleCharOnlyEvent(unsigned int charCode)
	{
		if (this->charCallback != NULL && charCode >= 32 && charCode != 127) // special hack, backspace induces a character in some implementations
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
				(*this->mouseDownCallback)(button);
			}
			// TODO - this is DEPRECATED
			else if (this->mouseDownCallback_DEPRECATED != NULL)
			{
				gvec2 cursorPosition = getCursorPosition();
				(*this->mouseDownCallback_DEPRECATED)(cursorPosition.x, cursorPosition.y, button);
			}
			break;
		case AMOUSEEVT_UP:
			if (this->mouseUpCallback != NULL)
			{
				(*this->mouseUpCallback)(button);
			}
			// TODO - this is DEPRECATED
			else if (this->mouseUpCallback_DEPRECATED != NULL)
			{
				gvec2 cursorPosition = getCursorPosition();
				(*this->mouseUpCallback_DEPRECATED)(cursorPosition.x, cursorPosition.y, button);
			}
			break;
		case AMOUSEEVT_MOVE:
			if (this->mouseMoveCallback != NULL)
			{
				(*this->mouseMoveCallback)();
			}
			// TODO - this is DEPRECATED
			else if (this->mouseMoveCallback_DEPRECATED != NULL)
			{
				gvec2 cursorPosition = getCursorPosition();
				(*this->mouseMoveCallback_DEPRECATED)(cursorPosition.x, cursorPosition.y);
			}
			break;
		case AMOUSEEVT_SCROLL:
			if (this->mouseScrollCallback != NULL)
			{
				(*this->mouseScrollCallback)(position.x, position.y);
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
	
}
