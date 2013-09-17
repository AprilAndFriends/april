/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT_WINDOW) && !defined(_WINP8)
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
		this->setCursorVisible(true);
		return true;
	}
	
	void WinRT_Window::unassign()
	{
		WinRT::App->unassignWindow();
		Window::unassign();
	}

	void WinRT_Window::setTitle(chstr title)
	{
		hlog::warn(april::logTag, "Window::setTitle() does nothing on WinRT.");
	}
	
	void WinRT_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		WinRT::App->setCursorVisible(value);
	}
	
	void* WinRT_Window::getBackendId()
	{
		// TODO ?
		return 0;
	}

	void WinRT_Window::setResolution(int w, int h, bool fullscreen)
	{
		((DirectX11_RenderSystem*)april::rendersys)->updateSwapChain();
		this->_setRenderSystemResolution(w, h, fullscreen);
		//hlog::error(april::logTag, "Cannot change resolution on window system: " + this->name);
	}
	
	void WinRT_Window::presentFrame()
	{
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
