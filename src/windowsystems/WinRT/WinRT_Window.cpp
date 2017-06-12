/// @file
/// @version 4.3
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

	bool WinRT_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		// WinRT window forces its own size
		float dpiRatio = WinRT::getDpiRatio();
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		w = hround(window->Bounds.Width * dpiRatio);
		h = hround(window->Bounds.Height * dpiRatio);
		// WinRT 8.1 or earlier is always considered fullscreen
		fullscreen = true;
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->delaySplash = 0.0f;
		this->backButtonSystemHandling = false;
		this->cursorMappings.clear();
		this->inputMode = InputMode::Touch;
		this->setCursorVisible(true);
		return true;
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
		return "";
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
		}
#else
		if (param == WINP8_BACK_BUTTON_SYSTEM_HANDLING)
		{
			this->backButtonSystemHandling = (value != "0");
		}
#endif
		if (param == WINRT_DELAY_SPLASH)
		{
			this->delaySplash = (float)value;
		}
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

	void WinRT_Window::setResolution(int w, int h, bool fullscreen)
	{
	}

	void WinRT_Window::changeSize(float w, float h)
	{
		april::getSystemInfo(); // so the displayResolution value gets updated
		float dpiRatio = WinRT::getDpiRatio();
		this->width = hround(w * dpiRatio);
		this->height = hround(h * dpiRatio);
		this->_setRenderSystemResolution();
	}
	
	void WinRT_Window::presentFrame()
	{
	}

	void WinRT_Window::terminateMainLoop()
	{
		this->running = false;
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
