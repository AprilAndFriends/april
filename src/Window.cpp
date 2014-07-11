/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "KeyboardDelegate.h"
#include "Keys.h"
#include "MouseDelegate.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "TouchDelegate.h"
#include "UpdateDelegate.h"
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
	
	Window::Window() : created(false), fullscreen(true), focused(true), running(true),
		fps(0), fpsCount(0), fpsTimer(0.0f), fpsResolution(0.5f), cursorVisible(false)
	{
		april::window = this;
		this->name = "Generic";
		this->updateDelegate = NULL;
		this->keyboardDelegate = NULL;
		this->mouseDelegate = NULL;
		this->touchDelegate = NULL;
		this->systemDelegate = NULL;
	}
	
	Window::~Window()
	{
		this->destroy();
	}

	bool Window::create(int w, int h, bool fullscreen, chstr title, chstr options)
	{
		if (!this->created)
		{
			hlog::writef(april::logTag, "Creating window: '%s' (%d, %d), '%s' fullscreen : %s",
				this->name.c_str(), w, h, title.c_str(), fullscreen ? "yes" : "no");
			this->fullscreen = fullscreen;
			this->title = title;
			this->created = true;
			this->fps = 0;
			this->fpsCount = 0;
			this->fpsTimer = 0.0f;
			this->fpsResolution = 0.5f;
			return true;
		}
		return false;
	}
	
	bool Window::destroy()
	{
		if (this->created)
		{
			hlog::writef(april::logTag, "Destroying window '%s'.", this->name.c_str());
			this->created = false;
			this->updateDelegate = NULL;
			this->keyboardDelegate = NULL;
			this->mouseDelegate = NULL;
			this->touchDelegate = NULL;
			this->systemDelegate = NULL;
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
	
	void Window::enterMainLoop()
	{
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
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
		static bool result;
		static float k;
		k = this->_calcTimeSinceLastFrame();
		if (!this->focused)
		{
			hthread::sleep(40.0f);
		}
		result = this->performUpdate(k);
		april::rendersys->presentFrame();
		return (result && this->running);
	}
	
	void Window::terminateMainLoop()
	{
		this->running = false;
	}
	
	bool Window::performUpdate(float k)
	{
		this->fpsTimer += k;
		if (this->fpsTimer > 0.0f)
		{
			this->fpsCount++;
			if (this->fpsTimer >= this->fpsResolution)
			{
				this->fps = (int)(this->fpsCount / this->fpsTimer);
				this->fpsCount = 0;
				this->fpsTimer = 0.0f;
			}
		}
		else
		{
			this->fps = 0;
			this->fpsCount = 0;
		}
		// returning true: continue execution
		// returning false: abort execution
		if (this->updateDelegate != NULL)
		{
			return this->updateDelegate->onUpdate(k);
		}
		april::rendersys->clear();
		return true;
	}
	
	void Window::handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode)
	{
		this->handleKeyOnlyEvent(type, keyCode);
		if (type == AKEYEVT_DOWN)
		{
			this->handleCharOnlyEvent(charCode);
		}
	}
	
	void Window::handleKeyOnlyEvent(KeyEventType type, Key keyCode)
	{
		if (keyCode == AK_UNKNOWN)
		{	
			keyCode = AK_NONE;
		}
		if (this->keyboardDelegate != NULL && keyCode != AK_NONE)
		{
			switch (type)
			{
			case AKEYEVT_DOWN:
				this->keyboardDelegate->onKeyDown(keyCode);
				break;
			case AKEYEVT_UP:
				this->keyboardDelegate->onKeyUp(keyCode);
				break;
			}
		}
	}
	
	void Window::handleCharOnlyEvent(unsigned int charCode)
	{
		if (this->keyboardDelegate != NULL && charCode >= 32 && charCode != 127) // special hack, backspace induces a character in some implementations
		{
			this->keyboardDelegate->onChar(charCode);
		}
	}
	
	void Window::handleMouseEvent(MouseEventType type, gvec2 position, Key button)
	{
		if (this->mouseDelegate != NULL)
		{
			switch (type)
			{
			case AMOUSEEVT_DOWN:
				this->mouseDelegate->onMouseDown(button);
				break;
			case AMOUSEEVT_UP:
				this->mouseDelegate->onMouseUp(button);
				break;
			case AMOUSEEVT_MOVE:
				this->mouseDelegate->onMouseMove();
				break;
			case AMOUSEEVT_SCROLL:
				this->mouseDelegate->onMouseScroll(position.x, position.y);
				break;
			}
		}
	}
	
	void Window::handleTouchEvent(const harray<gvec2>& touches)
	{
		if (this->touchDelegate != NULL)
		{
			this->touchDelegate->onTouch(touches);
		}
	}

	bool Window::handleQuitRequest(bool canCancel)
	{
		// returns whether or not the windowing system is permitted to close the window
		if (this->systemDelegate != NULL)
		{
			return this->systemDelegate->onQuit(canCancel);
		}
		return true;
	}
	
	void Window::handleFocusChangeEvent(bool focused)
	{
		this->focused = focused;
		hlog::write(april::logTag, "Window " + hstr(focused ? "activated" : "deactivated") + ".");
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onWindowFocusChanged(focused);
		}
	}
	
	bool Window::handleUrl(chstr url)
	{
		if (this->systemDelegate != NULL)
		{
			return this->systemDelegate->onHandleUrl(url);
		}
		return false;
	}
	
	void Window::handleLowMemoryWarning()
	{
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onLowMemoryWarning();
		}
	}

	float Window::_calcTimeSinceLastFrame()
	{
		float k = this->timer.diff(true);
		if (k > 0.5f)
		{
			k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
		}
		if (!this->focused)
		{
			k = 0.0f;
		}
		return k;
	}
	
}
