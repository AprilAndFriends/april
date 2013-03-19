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

	AndroidJNI_Window::AndroidJNI_Window() : Window(), width(0), height(0)
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
		this->inputMode = TOUCH;
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
	
	void AndroidJNI_Window::presentFrame()
	{
		// not needed as Android Java Activity takes care of this
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