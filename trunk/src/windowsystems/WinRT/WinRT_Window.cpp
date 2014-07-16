/// @file
/// @version 3.5
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
		this->name = APRIL_WS_WINRT;
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
		if (!Window::create(w, h, true, title, options))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->delaySplash = 0.0f;
		this->backButtonSystemHandling = false;
		this->cursorMappings.clear();
		this->inputMode = TOUCH;
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
				mappings += hsprintf("%u %s", it->second, it->first.c_str());
			}
			return mappings.join('\n');
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
		hlog::warn(april::logTag, "Window::setTitle() does nothing on WinRT.");
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

	void WinRT_Window::changeSize(int w, int h)
	{
		if (this->width != w || this->height != h)
		{
			this->width = w;
			this->height = h;
			this->_setRenderSystemResolution(w, h, this->fullscreen);
		}
	}
	
	void WinRT_Window::presentFrame()
	{
	}

	void WinRT_Window::beginKeyboardHandling()
	{
		WinRT::App->Overlay->showKeyboard();
	}

	void WinRT_Window::terminateKeyboardHandling()
	{
		WinRT::App->Overlay->hideKeyboard();
	}

	hstr WinRT_Window::findCursorFile(chstr filename)
	{
		if (filename != "")
		{
			foreach (hstr, it, this->cursorExtensions)
			{
				if (this->cursorMappings.has_key(filename))
				{
					return hstr(this->cursorMappings[filename]);
				}
			}
		}
		return "0";
	}

	Cursor* WinRT_Window::_createCursor()
	{
		return new WinRT_Cursor();
	}
	
	void WinRT_Window::_refreshCursor()
	{
		WinRT::App->refreshCursor();
	}
	
}
#endif
