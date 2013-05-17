/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE_WINDOW
#include <KD/kd.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"

#ifdef _EGL
#include "egl.h"
#endif
#include "OpenGL_RenderSystem.h"
#include "OpenKODE_Window.h"

namespace april
{
	OpenKODE_Window::OpenKODE_Window() : Window()
	{
		this->name = APRIL_WS_OPENKODE;
		this->kdWindow = NULL;
#if defined(_WIN32) && !defined(_EGL)
		hlog::warn(april::logTag, "OpenKODE Window requires EGL to be present!");
#endif
	}

	OpenKODE_Window::~OpenKODE_Window()
	{
		this->destroy();
	}

	bool OpenKODE_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
#ifdef _EGL
		april::egl.init();
#endif
		this->kdWindow = kdCreateWindow(april::egl.display, april::egl.config, NULL);
		if (this->kdWindow == NULL)
		{
			hlog::error(april::logTag, "Can't create KD Window!");
			return false;
		}
		if (kdRealizeWindow(this->kdWindow, &april::egl.hWnd) != NULL)
		{
			hlog::error(april::logTag, "Can't realize KD Window!");
			this->destroy();
			return false;
		}
#ifdef _EGL
		april::egl.create();
#endif
		if (w != 0 && h != 0)
		{
			KDint32 size[] = {w, h};
			kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		}
		kdSetWindowPropertycv(this->kdWindow, KD_WINDOWPROPERTY_CAPTION, title.c_str());
		return true;
	}
	
	bool OpenKODE_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		april::egl.destroy();
		if (this->kdWindow != NULL)
		{
			kdDestroyWindow(this->kdWindow);
			this->kdWindow = NULL;
		}
		return true;
	}

	int OpenKODE_Window::getWidth()
	{
		KDint32 size[] = {0, 0};
		kdGetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		return size[0];
	}
	
	int OpenKODE_Window::getHeight()
	{
		KDint32 size[] = {0, 0};
		kdGetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		return size[1];
	}
	
	void OpenKODE_Window::setTitle(chstr title)
	{
		this->title = title;
		kdSetWindowPropertycv(this->kdWindow, KD_WINDOWPROPERTY_CAPTION, this->title.c_str());
	}
	
	bool OpenKODE_Window::isCursorVisible()
	{
		return (Window::isCursorVisible() || !this->isCursorInside());
	}
	
	void OpenKODE_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
	}
	
	void* OpenKODE_Window::getBackendId()
	{
#ifdef _EGL
		return april::egl.hWnd;
#else
		return 0;
#endif
	}

	void OpenKODE_Window::_setResolution(int w, int h)
	{
		// TODO
	}
	
	bool OpenKODE_Window::updateOneFrame()
	{
		static bool result;
		this->checkEvents();
		// rendering
		result = Window::updateOneFrame();
		this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
		return result;
	}
	
	void OpenKODE_Window::presentFrame()
	{
#ifdef _EGL
		april::egl.swapBuffers();
#endif
	}

	bool __process_event(const KDEvent* evt)
	{
		gvec2 cursorPosition;
		switch (evt->type)
		{
		case KD_EVENT_QUIT:
			april::window->handleQuitRequest(false);
			april::window->terminateMainLoop();
			return true;
		case KD_EVENT_WINDOW_CLOSE:
			april::window->handleQuitRequest(true);
			return true;
		case KD_EVENT_PAUSE:
			april::window->handleFocusChangeEvent(false);
#ifdef _EGL
			april::egl.destroy();
#endif
			return true;
		case KD_EVENT_RESUME:
#ifdef _EGL
			april::egl.create();
#endif
			april::window->handleFocusChangeEvent(true);
			return true;
		case KD_EVENT_WINDOW_FOCUS:
			april::window->handleFocusChangeEvent(evt->data.windowfocus.focusstate != 0);
			return true;
		case KD_EVENT_INPUT_POINTER:
			cursorPosition.set((float)evt->data.inputpointer.x, (float)evt->data.inputpointer.y);
			((OpenKODE_Window*)april::window)->setCursorPosition(cursorPosition);
			switch (evt->data.inputpointer.index)
			{
			case KD_INPUT_POINTER_X:
			case KD_INPUT_POINTER_Y:
				april::window->queueTouchEvent(Window::AMOUSEEVT_MOVE, cursorPosition, 0);
				break;
			case KD_INPUT_POINTER_SELECT:
				if (evt->data.inputpointer.select != 0)
				{
					april::window->queueTouchEvent(Window::AMOUSEEVT_DOWN, cursorPosition, 0);
				}
				else
				{
					april::window->queueTouchEvent(Window::AMOUSEEVT_UP, cursorPosition, 0);
				}
				break;
			}
			return true;
		case KD_EVENT_ORIENTATION:
			// TODO
			//april::window->getSystemDelegate()->onWindowSizeChanged();
			return false;
		}
		return false;
	}
	
	void OpenKODE_Window::checkEvents()
	{
		kdPumpEvents();
		const KDEvent* evt;
		// 1 milisecond as timeout
		while (this->running && (evt = kdWaitEvent(1000000L)) != NULL)
		{
			if (!__process_event(evt))
			{
				kdDefaultEvent(evt);
			}
		}
		Window::checkEvents();
	}

}
#endif