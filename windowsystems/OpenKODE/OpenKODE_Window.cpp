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
		this->virtualKeyboardVisible = false;
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
		this->virtualKeyboardVisible = false;
		if (w <= 0 || h <= 0)
		{
			hlog::errorf(april::logTag, "Cannot create window with size: %d x %d", w, h);
			this->destroy();
			return false;
		}
#ifdef _EGL
		this->kdWindow = kdCreateWindow(april::egl->display, april::egl->config, NULL);
#endif
		if (this->kdWindow == NULL)
		{
			hlog::error(april::logTag, "Can't create KD Window!");
			this->destroy();
			return false;
		}
		if (fullscreen) // KD only supports fullscreen in the screen's resolution
		{
			kdQueryAttribi(KD_ATTRIB_WIDTH, &w);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, &h);
		}
		KDint32 size[] = {w, h};
		kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
		kdSetWindowPropertycv(this->kdWindow, KD_WINDOWPROPERTY_CAPTION, title.c_str());
#ifdef _EGL // KD doesn't actually work without EGL
		if (kdRealizeWindow(this->kdWindow, &april::egl->hWnd) != 0)
#endif
		{
			hlog::error(april::logTag, "Can't realize KD Window!");
			this->destroy();
			return false;
		}
#ifdef _EGL
		april::egl->create();
#endif
		return true;
	}
	
	bool OpenKODE_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		this->virtualKeyboardVisible = false;
#ifdef _EGL
		april::egl->destroy();
#endif
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
		return april::egl->hWnd;
#else
		return 0;
#endif
	}

	void OpenKODE_Window::setResolution(int w, int h, bool fullscreen)
	{
		if (fullscreen) // KD only supports fullscreen in the screen's resolution
		{
			kdQueryAttribi(KD_ATTRIB_WIDTH, &w);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, &h);
		}
		this->fullscreen = fullscreen;
		KDint32 size[] = {w, h};
		kdSetWindowPropertyiv(this->kdWindow, KD_WINDOWPROPERTY_SIZE, size);
	}

	void OpenKODE_Window::handleActivityChangeEvent(bool active)
	{
		this->handleFocusChangeEvent(active);
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
		april::egl->swapBuffers();
#endif
	}

	bool OpenKODE_Window::_processEvent(const KDEvent* evt)
	{
		switch (evt->type)
		{
		case KD_EVENT_QUIT:
			this->handleQuitRequest(false);
			this->terminateMainLoop();
			return true;
		case KD_EVENT_WINDOW_CLOSE:
			this->handleQuitRequest(true);
			return true;
		case KD_EVENT_PAUSE:
			this->handleActivityChangeEvent(false);
			april::rendersys->unloadTextures();
#ifdef _EGL
			april::egl->destroy();
#endif
			return true;
		case KD_EVENT_RESUME:
#ifdef _EGL
			april::egl->create();
#endif
			this->handleActivityChangeEvent(true);
			return true;
		case KD_EVENT_WINDOW_FOCUS:
			this->handleFocusChangeEvent(evt->data.windowfocus.focusstate != 0);
			return true;
		case KD_EVENT_INPUT:
			if (evt->data.input.value.i != 0)
			{
				if (evt->data.input.index < KD_IOGROUP_CHARS) // because key and char events are separate
				{
					this->queueKeyEvent(april::Window::AKEYEVT_DOWN, (april::Key)evt->data.input.index, 0);
				}
				else
				{
					this->queueKeyEvent(april::Window::AKEYEVT_DOWN, april::AK_NONE, evt->data.input.index - KD_IOGROUP_CHARS);
				}
			}
			else
			{
				this->queueKeyEvent(april::Window::AKEYEVT_UP, (april::Key)evt->data.input.index, 0);
			}
			return true;
		case KD_EVENT_INPUT_POINTER:
			this->cursorPosition.set((float)evt->data.inputpointer.x, (float)evt->data.inputpointer.y);
			switch (evt->data.inputpointer.index)
			{
			case KD_INPUT_POINTER_X:
			case KD_INPUT_POINTER_Y:
				this->queueTouchEvent(Window::AMOUSEEVT_MOVE, this->cursorPosition, 0);
				break;
			case KD_INPUT_POINTER_SELECT:
				if (evt->data.inputpointer.select != 0)
				{
					this->queueTouchEvent(Window::AMOUSEEVT_DOWN, this->cursorPosition, 0);
				}
				else
				{
					this->queueTouchEvent(Window::AMOUSEEVT_UP, this->cursorPosition, 0);
				}
				break;
			}
			return true;
		case KD_EVENT_WINDOWPROPERTY_CHANGE:
			if (evt->data.windowproperty.pname == KD_WINDOWPROPERTY_SIZE)
			{
				this->_setRenderSystemResolution();
			}
			return true;
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
			if (!this->_processEvent(evt))
			{
				kdDefaultEvent(evt);
			}
		}
		Window::checkEvents();
	}

	void OpenKODE_Window::beginKeyboardHandling()
	{
		kdKeyboardShow(this->kdWindow, 1);
		this->virtualKeyboardVisible = true;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(true);
		}
	}

	void OpenKODE_Window::terminateKeyboardHandling()
	{
		kdKeyboardShow(this->kdWindow, 0);
		this->virtualKeyboardVisible = false;
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(false);
		}
	}

}
#endif