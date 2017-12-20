/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdlib.h>

#ifdef _ANDROIDJNI_WINDOW
#include <jni.h>

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#define __NATIVE_INTERFACE_CLASS "com/april/NativeInterface"
#include "androidUtilJNI.h"
#include "AndroidJNI_Cursor.h"
#include "AndroidJNI_Window.h"
#include "AndroidJNI_Keys.h"
#include "april.h"
#include "MotionDelegate.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"

namespace april
{
	extern void* javaVM;

	AndroidJNI_Window::AndroidJNI_Window() : Window(), width(0), height(0), manualPresentFrameEnabled(true), forcedFocus(false)
	{
		this->name = april::WindowType::AndroidJNI.getName();
		initAndroidKeyMap();
	}

	AndroidJNI_Window::~AndroidJNI_Window()
	{
		this->destroy();
	}

	bool AndroidJNI_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, true, title, options))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->inputMode = InputMode::Touch;
		this->forcedFocus = false;
		return true;
	}
	
	void* AndroidJNI_Window::getBackendId() const
	{
		return javaVM;
	}

	void AndroidJNI_Window::enterMainLoop()
	{
		hlog::error(logTag, "Using enterMainLoop() on Android JNI is not valid!");
		exit(-1);
	}
	
	void AndroidJNI_Window::presentFrame()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodSwapBuffers, "swapBuffers", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodSwapBuffers);
		env->PopLocalFrame(NULL);
	}

	bool AndroidJNI_Window::updateOneFrame()
	{
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jmethodID methodSetSensorsEnabled = env->GetStaticMethodID(classNativeInterface, "setSensorsEnabled", _JARGS(_JVOID, _JBOOL _JBOOL _JBOOL _JBOOL _JBOOL));
		if (methodSetSensorsEnabled != NULL)
		{
			if (this->motionDelegate != NULL)
			{
				env->CallStaticVoidMethod(classNativeInterface, methodSetSensorsEnabled,
					this->motionDelegate->isAccelerometerEnabled(), this->motionDelegate->isLinearAccelerometerEnabled(),
					this->motionDelegate->isGravityEnabled(), this->motionDelegate->isRotationEnabled(), this->motionDelegate->isGyroscopeEnabled());
			}
			else
			{
				env->CallStaticVoidMethod(classNativeInterface, methodSetSensorsEnabled, JNI_FALSE, JNI_FALSE, JNI_FALSE, JNI_FALSE, JNI_FALSE);
			}
		}
		else
		{
			hlog::error(APRIL_JNI_LOG_TAG, "Could not find method, check definition: setSensorsEnabled");
		}
		env->PopLocalFrame(NULL);
		return Window::updateOneFrame();
	}

	void AndroidJNI_Window::queueTouchEvent(MouseInputEvent::Type type, cgvec2 position, int index)
	{
		if (type == MouseInputEvent::Type::Down || type == MouseInputEvent::Type::Up)
		{
			this->setInputMode(InputMode::Touch);
		}
		Window::queueTouchEvent(type, position, index);
	}

	void AndroidJNI_Window::queueControllerEvent(ControllerInputEvent::Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		this->setInputMode(InputMode::Controller);
		Window::queueControllerEvent(type, controllerIndex, buttonCode, axisValue);
	}

	void AndroidJNI_Window::showVirtualKeyboard()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodShowVirtualKeyboard, "showVirtualKeyboard", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodShowVirtualKeyboard);
		env->PopLocalFrame(NULL);
	}
	
	void AndroidJNI_Window::hideVirtualKeyboard()
	{
		if (this->manualPresentFrameEnabled)
		{
			APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodHideVirtualKeyboard, "hideVirtualKeyboard", _JARGS(_JVOID, ));
			env->CallStaticVoidMethod(classNativeInterface, methodHideVirtualKeyboard);
			env->PopLocalFrame(NULL);
		}
	}

	void AndroidJNI_Window::handleFocusChangeEvent(bool focused)
	{
		this->forcedFocus = false;
		Window::handleFocusChangeEvent(focused);
	}

	void AndroidJNI_Window::handleActivityChange(bool active)
	{
		if (!active)
		{
			if (this->focused)
			{
				this->forcedFocus = true;
				Window::handleFocusChangeEvent(false);
			}
		}
		else if (this->forcedFocus)
		{
			// only return focus if previously lost focus through acitvity state change
			this->forcedFocus = false;
			Window::handleFocusChangeEvent(true);
		}
	}

	Cursor* AndroidJNI_Window::_createCursor(bool fromResource)
	{
		return new AndroidJNI_Cursor(fromResource);
	}

}
#endif
