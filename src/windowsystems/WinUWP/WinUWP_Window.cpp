/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINUWP_WINDOW
#include <hltypes/hfile.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hrdir.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX12_RenderSystem.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"
#include "WinUWP.h"
#include "WinUWP_Cursor.h"
#include "WinUWP_Window.h"

using namespace Windows::UI::ViewManagement;

#define DX12_RENDERSYS ((DirectX12_RenderSystem*)rendersys)

namespace april
{
	WinUWP_Window::WinUWP_Window() : Window()
	{
		this->name = april::WindowType::WinUWP.getName();
		this->width = 0;
		this->height = 0;
		this->delaySplash = 0.0f;
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
	}

	WinUWP_Window::~WinUWP_Window()
	{
		this->destroy();
	}

	bool WinUWP_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		Rect rect = CoreWindow::GetForCurrentThread()->Bounds;
		w = (int)rect.Width;
		h = (int)rect.Height;
		fullscreen = ApplicationView::GetForCurrentView()->IsFullScreenMode; // WinUWP is always fullscreen
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
	
	hstr WinUWP_Window::getParam(chstr param)
	{
#ifndef _WINP8
		if (param == WINUWP_CURSOR_MAPPINGS)
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
		if (param == WINUWP_DELAY_SPLASH)
		{
			return hstr(this->delaySplash);
		}
		return "";
	}

	void WinUWP_Window::setParam(chstr param, chstr value)
	{
#ifndef _WINP8
		if (param == WINUWP_CURSOR_MAPPINGS)
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
		if (param == WINUWP_DELAY_SPLASH)
		{
			this->delaySplash = (float)value;
		}
	}

	void WinUWP_Window::setTitle(chstr title)
	{
		hlog::warn(logTag, "Window::setTitle() does nothing on WinRT.");
	}
	
	void* WinUWP_Window::getBackendId() const
	{
		// TODO ?
		return 0;
	}

	void WinUWP_Window::setResolution(int w, int h, bool fullscreen)
	{
		this->width = w;
		this->height = h;
		this->_setRenderSystemResolution(w, h, fullscreen);
	}

	void WinUWP_Window::presentFrame()
	{
	}

	bool WinUWP_Window::updateOneFrame()
	{
		ID3D12CommandQueue* commandQueue = DX12_RENDERSYS->getCommandQueue();
		PIXBeginEvent(commandQueue, 0, L"updateOneFrame()");
		bool result = Window::updateOneFrame();
		PIXEndEvent(commandQueue);
		return result;
	}

	void WinUWP_Window::checkEvents()
	{
		// TODOuwp - implement this
		/*
		if (this->visible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
		*/
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		Window::checkEvents();
	}

	void WinUWP_Window::terminateMainLoop()
	{
		this->running = false;
	}

	void WinUWP_Window::showVirtualKeyboard()
	{
		//WinUWP::App->Overlay->showKeyboard();
	}

	void WinUWP_Window::hideVirtualKeyboard()
	{
		//WinUWP::App->Overlay->hideKeyboard();
	}

	hstr WinUWP_Window::findCursorFile(chstr filename) const
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

	Cursor* WinUWP_Window::_createCursor(bool fromResource)
	{
		return new WinUWP_Cursor(fromResource);
	}
	
	void WinUWP_Window::_refreshCursor()
	{
		WinUWP::App->refreshCursor();
	}
	
}
#endif
