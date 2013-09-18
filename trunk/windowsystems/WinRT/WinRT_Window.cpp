/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW
#include <hltypes/hfile.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"
#include "WinRT.h"
#include "WinRT_Window.h"

namespace april
{
	WinRT_Window::WinRT_Window() : Window()
	{
		this->name = APRIL_WS_WINRT;
		this->width = 0;
		this->height = 0;
	}

	WinRT_Window::~WinRT_Window()
	{
		this->destroy();
	}

	bool WinRT_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->inputMode = TOUCH;
		this->setCursorVisible(true);
		return true;
	}
	
	void WinRT_Window::unassign()
	{
		WinRT::Interface->unassignWindow();
		Window::unassign();
	}

	void WinRT_Window::setTitle(chstr title)
	{
		hlog::warn(april::logTag, "Window::setTitle() does nothing on WinRT.");
	}
	
	void WinRT_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		WinRT::Interface->setCursorVisible(value);
	}
	
	void* WinRT_Window::getBackendId()
	{
		// TODO ?
		return 0;
	}

	void WinRT_Window::setResolution(int w, int h, bool fullscreen)
	{
		hlog::error(april::logTag, "Cannot change resolution on window system: " + this->name);
	}
	
	void WinRT_Window::presentFrame()
	{
	}

	void WinRT_Window::checkEvents()
	{
		Window::checkEvents();
		WinRT::Interface->checkEvents();
	}
	
	void WinRT_Window::beginKeyboardHandling()
	{
		WinRT::Interface->showKeyboard();
		this->virtualKeyboardVisible = true;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(true);
		}
	}

	void WinRT_Window::terminateKeyboardHandling()
	{
		WinRT::Interface->hideKeyboard();
		this->virtualKeyboardVisible = false;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(false);
		}
	}

}
#endif
