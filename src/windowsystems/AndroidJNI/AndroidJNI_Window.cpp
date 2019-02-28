/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _ANDROIDJNI_WINDOW
#include <stdlib.h>
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

	AndroidJNI_Window::AndroidJNI_Window() :
		Window(),
		width(0),
		height(0),
		forcedFocus(false)
	{
		this->name = april::WindowType::AndroidJNI.getName();
		initAndroidKeyMap();
	}

	AndroidJNI_Window::~AndroidJNI_Window()
	{
		this->destroy();
	}

	void AndroidJNI_Window::_systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		if (options.minimized)
		{
			options.minimized = false;
			hlog::warn(logTag, "Option 'minimized' is not supported on window system: " + this->name);
		}
		Window::_systemCreate(width, height, true, title, options);
		this->width = width;
		this->height = height;
		this->inputMode = InputMode::Touch;
		this->forcedFocus = false;
	}
	
	void* AndroidJNI_Window::getBackendId() const
	{
		return javaVM;
	}

	bool AndroidJNI_Window::update(float timeDelta)
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
		return Window::update(timeDelta);
	}

	void AndroidJNI_Window::_presentFrame(bool systemEnabled)
	{
		Window::_presentFrame(systemEnabled);
		if (systemEnabled)
		{
			APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodSwapBuffers, "swapBuffers", _JARGS(_JVOID, ));
			env->CallStaticVoidMethod(classNativeInterface, methodSwapBuffers);
			env->PopLocalFrame(NULL);
		}
	}

	void AndroidJNI_Window::queueTouchInput(TouchEvent::Type type, int index, cgvec2f position)
	{
		if (type == TouchEvent::Type::Down || type == TouchEvent::Type::Up)
		{
			this->queueInputModeChange(InputMode::Touch);
		}
		Window::queueTouchInput(type, index, position);
	}

	void AndroidJNI_Window::queueControllerInput(ControllerEvent::Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		this->queueInputModeChange(InputMode::Controller);
		Window::queueControllerInput(type, controllerIndex, buttonCode, axisValue);
	}

	void AndroidJNI_Window::showVirtualKeyboard()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodShowVirtualKeyboard, "showVirtualKeyboard", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodShowVirtualKeyboard);
		env->PopLocalFrame(NULL);
	}
	
	void AndroidJNI_Window::hideVirtualKeyboard()
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodHideVirtualKeyboard, "hideVirtualKeyboard", _JARGS(_JVOID, ));
		env->CallStaticVoidMethod(classNativeInterface, methodHideVirtualKeyboard);
		env->PopLocalFrame(NULL);
	}

	void AndroidJNI_Window::handleFocusChange(bool focused)
	{
		this->forcedFocus = false;
		Window::handleFocusChange(focused);
	}

	void AndroidJNI_Window::handleActivityChange(bool active)
	{
		if (!active)
		{
			if (this->focused)
			{
				this->forcedFocus = true;
				Window::handleFocusChange(false);
			}
		}
		else if (this->forcedFocus)
		{
			// only return focus if previously lost focus through acitvity state change
			this->forcedFocus = false;
			Window::handleFocusChange(true);
		}
	}

	Cursor* AndroidJNI_Window::_createCursor(bool fromResource)
	{
		return new AndroidJNI_Cursor(fromResource);
	}

}
#endif
