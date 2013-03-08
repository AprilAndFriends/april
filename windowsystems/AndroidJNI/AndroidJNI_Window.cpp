/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
#include "AndroidJNI_Window.h"
#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"

#define CLASS_VIEW "android/view/View"
#define CLASS_IBINDER "android/os/IBinder"

namespace april
{
	extern JavaVM* javaVM;

	AndroidJNI_Window::AndroidJNI_Window() : Window(), width(0), height(0), multiTouchActive(false)
	{
		this->name = APRIL_WS_ANDROIDJNI;
	}

	AndroidJNI_Window::~AndroidJNI_Window()
	{
		this->destroy();
	}

	bool AndroidJNI_Window::create(int w, int h, bool fullscreen, chstr title, chstr options)
	{
		if (!Window::create(w, h, true, title, options))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->multiTouchActive = false;
		return true;
	}
	
	void* AndroidJNI_Window::getBackendId()
	{
		return javaVM;
	}

	void AndroidJNI_Window::enterMainLoop()
	{
		hlog::error(april::logTag, "Using enterMainLoop on Android JNI!");
		exit(-1);
	}
	
	bool AndroidJNI_Window::updateOneFrame()
	{
		this->checkEvents();
		return Window::updateOneFrame();
	}
	
	void AndroidJNI_Window::presentFrame()
	{
		// not needed as Android Java Activity takes care of this
	}

	void AndroidJNI_Window::checkEvents()
	{
		while (this->mouseEvents.size() > 0)
		{
			MouseInputEvent e = this->mouseEvents.remove_first();
			this->cursorPosition = e.position;
			Window::handleMouseEvent(e.type, e.position, e.button);
		}
		while (this->keyEvents.size() > 0)
		{
			KeyInputEvent e = this->keyEvents.remove_first();
			Window::handleKeyEvent(e.type, e.keyCode, e.charCode);
		}
		while (this->touchEvents.size() > 0)
		{
			TouchInputEvent e = this->touchEvents.remove_first();
			Window::handleTouchEvent(e.touches);
		}
	}

	void AndroidJNI_Window::handleTouchEvent(MouseEventType type, gvec2 position, int index)
	{
		int previousTouchesSize = this->touches.size();
		switch (type)
		{
		case AMOUSEEVT_DOWN:
			if (index < this->touches.size()) // DOWN event of an already indexed touch, never happened so far
			{
				return;
			}
			this->touches += position;
			break;
		case AMOUSEEVT_UP:
			if (index >= this->touches.size()) // redundant UP event, can happen
			{
				return;
			}
			this->touches.remove_at(index);
			break;
		case AMOUSEEVT_MOVE:
			if (index >= this->touches.size()) // MOVE event of an unindexed touch, never happened so far
			{
				return;
			}
			this->touches[index] = position;
			break;
		}
		if (this->multiTouchActive || this->touches.size() > 1)
		{
			if (!this->multiTouchActive && previousTouchesSize == 1)
			{
				// cancel (notify the app) the previously called mousedown event so we can begin the multi touch event properly
				this->handleMouseEvent(AMOUSEEVT_UP, gvec2(-10000.0f, -10000.0f), AK_LBUTTON);
			}
			this->multiTouchActive = (this->touches.size() > 0);
		}
		else
		{
			this->handleMouseEvent(type, position, AK_LBUTTON);
		}
		this->touchEvents.clear();
		this->touchEvents += TouchInputEvent(this->touches);
	}

	void AndroidJNI_Window::handleMouseEvent(MouseEventType type, gvec2 position, Key button)
	{
		this->mouseEvents += MouseInputEvent(type, position, button);
	}

	void AndroidJNI_Window::handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode)
	{
		this->keyEvents += KeyInputEvent(type, keyCode, charCode);
	}

	void AndroidJNI_Window::beginKeyboardHandling()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodShowVirtualKeyboard, "showVirtualKeyboard", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodShowVirtualKeyboard);
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(true);
		}
	}
	
	void AndroidJNI_Window::terminateKeyboardHandling()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodHideVirtualKeyboard, "hideVirtualKeyboard", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodHideVirtualKeyboard);
		if (this->systemDelegate != NULL)
		{
			this->systemDelegate->onVirtualKeyboardVisibilityChanged(false);
		}
	}

}
#endif