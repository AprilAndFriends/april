/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UWP_WINDOW
#include <hltypes/hfile.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hrdir.h>
#include <hltypes/hthread.h>

#include "Application.h"
#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"
#include "UWP.h"
#include "UWP_Cursor.h"
#include "UWP_Window.h"

using namespace Windows::Foundation;
using namespace Windows::UI::ViewManagement;

namespace april
{
	UWP_Window::UWP_Window() :
		Window()
	{
		this->name = april::WindowType::UWP.getName();
		this->width = 0;
		this->height = 0;
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
		this->backendId = NULL;
	}

	UWP_Window::~UWP_Window()
	{
		this->destroy();
	}

	void UWP_Window::_systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		if (options.minimized)
		{
			options.minimized = false;
			hlog::warn(logTag, "Option 'minimized' is not supported on window system: " + this->name);
		}
		if (!options.resizable)
		{
			options.resizable = true;
			hlog::warn(logTag, "Option 'resizable' cannot be turned off on window system: " + this->name);
		}
		ApplicationView^ view = ApplicationView::GetForCurrentView();
		view->PreferredLaunchWindowingMode = (fullscreen ? ApplicationViewWindowingMode::FullScreen : ApplicationViewWindowingMode::PreferredLaunchViewSize);
		view->PreferredLaunchViewSize = Size((float)width, (float)height);
		Window::_systemCreate(width, height, fullscreen, title, options);
		Rect rect = CoreWindow::GetForCurrentThread()->Bounds;
		float dpiRatio = UWP::getDpiRatio();
		this->width = (int)(rect.Width * dpiRatio);
		this->height = (int)(rect.Height * dpiRatio);
		this->backButtonSystemHandling = false;
		this->cursorMappings.clear();
		this->inputMode = InputMode::Touch;
		this->backendId = reinterpret_cast<IUnknown*>(CoreWindow::GetForCurrentThread());
		this->setCursorVisible(true);
		if (this->width != width || this->height != height)
		{
			this->queueSizeChange(this->width, this->height, fullscreen);
		}
		return;
	}
	
	hstr UWP_Window::getParam(chstr param)
	{
		if (param == UWP_CURSOR_MAPPINGS)
		{
			harray<hstr> mappings;
			foreach_m (int, it, this->cursorMappings)
			{
				mappings += hsprintf("%u %s", it->second, it->first.cStr());
			}
			return mappings.joined('\n');
		}
		return "";
	}

	void UWP_Window::setParam(chstr param, chstr value)
	{
		if (param == UWP_CURSOR_MAPPINGS)
		{
			this->cursorMappings.clear();
			harray<hstr> lines = value.split('\n', -1, true);
			harray<hstr> data;
			foreach (hstr, it, lines)
			{
				data = (*it).split(' ', 1);
				if (data.size() == 2)
				{
					this->cursorMappings[data[1]] = (int)data[0];
				}
			}
			return;
		}
		Window::setParam(param, value);
	}

	void UWP_Window::setTitle(chstr title)
	{
		if (this->options.fpsCounter)
		{
			hstr newTitle = title + hsprintf(" [FPS: %d]", april::application->getFps());
			// optimization to prevent setting title every frame
			if (newTitle == this->fpsTitle)
			{
				return;
			}
			this->fpsTitle = newTitle;
			UWP::app->setWindowTitle(_HL_HSTR_TO_PSTR(newTitle));
		}
		else
		{
			UWP::app->setWindowTitle(_HL_HSTR_TO_PSTR(title));
		}
		this->title = title;
	}
	
	bool UWP_Window::update(float timeDelta)
	{
		bool result = Window::update(timeDelta);
		if (this->options.fpsCounter)
		{
			this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
		}
		return result;
	}

	void UWP_Window::_systemSetResolution(int width, int height, bool fullscreen)
	{
		this->width = width;
		this->height = height;
		this->fullscreen = fullscreen;
		this->_setRenderSystemResolution(this->width, this->height, this->fullscreen);
	}

	void UWP_Window::checkEvents()
	{
		if (UWP::app->isVisible())
		{
			UWP::app->updateMainThread();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
		Window::checkEvents();
	}

	void UWP_Window::showVirtualKeyboard()
	{
		UWP::app->showVirtualKeyboard();
	}

	void UWP_Window::hideVirtualKeyboard()
	{
		UWP::app->hideVirtualKeyboard();
	}

	hstr UWP_Window::findCursorFile(chstr filename) const
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

	Cursor* UWP_Window::_createCursor(bool fromResource)
	{
		return new UWP_Cursor(fromResource);
	}
	
	void UWP_Window::_refreshCursor()
	{
		UWP::app->refreshCursor();
	}
	
}
#endif
