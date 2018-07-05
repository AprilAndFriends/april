/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT_WINDOW
#include <hltypes/hfile.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hrdir.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"
#include "WinRT.h"
#include "WinRT_Cursor.h"
#include "WinRT_Window.h"

namespace april
{
	WinRT_Window::WinRT_Window() : Window()
	{
		this->name = april::WindowType::WinRT.getName();
		this->width = 0;
		this->height = 0;
		this->delaySplash = 0.0f;
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
	}

	WinRT_Window::~WinRT_Window()
	{
		this->destroy();
	}

	void WinRT_Window::_systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		if (options.minimized)
		{
			options.minimized = false;
			hlog::warn(logTag, "Option 'minimized' is not supported on window system: " + this->name);
		}
		// WinRT window forces its own size
		float dpiRatio = WinRT::getDpiRatio();
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		width = hround(window->Bounds.Width * dpiRatio);
		height = hround(window->Bounds.Height * dpiRatio);
		// WinRT 8.1 or earlier is always considered fullscreen
		fullscreen = true;
		Window::_systemCreate(width, height, fullscreen, title, options);
		this->width = width;
		this->height = height;
		this->delaySplash = 0.0f;
		this->backButtonSystemHandling = false;
		this->cursorMappings.clear();
		this->inputMode = InputMode::Touch;
		this->setCursorVisible(true);
	}
	
	hstr WinRT_Window::getParam(chstr param)
	{
#ifndef _WINP8
		if (param == WINRT_CURSOR_MAPPINGS)
		{
			harray<hstr> mappings;
			foreach_m (unsigned int, it, this->cursorMappings)
			{
				mappings += hsprintf("%u %s", it->second, it->first.cStr());
			}
			return mappings.joined('\n');
		}
#else
		if (param == WINP8_BACK_BUTTON_SYSTEM_HANDLING)
		{
			return hstr(this->backButtonSystemHandling ? "1" : "0");
		}
#endif
		if (param == WINRT_DELAY_SPLASH)
		{
			return hstr(this->delaySplash);
		}
		return Window::getParam(param);
	}

	void WinRT_Window::setParam(chstr param, chstr value)
	{
#ifndef _WINP8
		if (param == WINRT_CURSOR_MAPPINGS)
		{
			this->cursorMappings.clear();
			harray<hstr> lines = value.split('\n', -1, true);
			harray<hstr> data;
			foreach (hstr, it, lines)
			{
				data = (*it).split(' ', 1);
				if (data.size() == 2)
				{
					this->cursorMappings[data[1]] = (unsigned int)data[0];
				}
			}
			return;
		}
#else
		if (param == WINP8_BACK_BUTTON_SYSTEM_HANDLING)
		{
			this->backButtonSystemHandling = (value != "0");
			return;
		}
#endif
		if (param == WINRT_DELAY_SPLASH)
		{
			this->delaySplash = (float)value;
			return;
		}
		Window::setParam(param, value);
	}

	void WinRT_Window::setTitle(chstr title)
	{
		hlog::warn(logTag, "Window::setTitle() does nothing on WinRT.");
	}
	
	void* WinRT_Window::getBackendId() const
	{
		// TODO ?
		return 0;
	}

	void WinRT_Window::_systemSetResolution(int width, int height, bool fullscreen)
	{
		this->width = width;
		this->height = height;
		this->fullscreen = this->fullscreen;
		this->_setRenderSystemResolution(this->width, this->height, this->fullscreen);
	}

	void WinRT_Window::changeSize(float width, float height)
	{
		// TODOa - is this still ok here?
		hlog::write(logTag, "april::getSystemInfo() in WinRT_Window::changeSize()");
		april::getSystemInfo(); // so the displayResolution value gets updated
		float dpiRatio = WinRT::getDpiRatio();
		this->width = hround(width * dpiRatio);
		this->height = hround(height * dpiRatio);
		this->_setRenderSystemResolution();
	}
	
	void WinRT_Window::showVirtualKeyboard()
	{
		WinRT::App->Overlay->showKeyboard();
	}

	void WinRT_Window::hideVirtualKeyboard()
	{
		WinRT::App->Overlay->hideKeyboard();
	}

	hstr WinRT_Window::findCursorFile(chstr filename) const
	{
		if (filename != "")
		{
			foreachc (hstr, it, this->cursorExtensions)
			{
				if (this->cursorMappings.hasKey(filename))
				{
					return hstr(this->cursorMappings.tryGet(filename, 0));
				}
			}
		}
		return "0";
	}

	Cursor* WinRT_Window::_createCursor(bool fromResource)
	{
		return new WinRT_Cursor(fromResource);
	}
	
	void WinRT_Window::_refreshCursor()
	{
		WinRT::App->refreshCursor();
	}
	
}
#endif
