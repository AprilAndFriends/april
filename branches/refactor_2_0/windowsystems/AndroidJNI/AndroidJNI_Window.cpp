/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#include "AndroidJNI_Window.h"
#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	void* javaVM = NULL;
	jobject aprilActivity = NULL;
	gvec2 androidResolution;

	AndroidJNI_Window::AndroidJNI_Window(int width, int height, bool fullscreen, chstr title) : Window()
	{
		if (april::rendersys != NULL)
		{
			april::log("Creating Android JNI Windowsystem (" + hstr(width) + ", " + hstr(height) + ")");
		}
		this->width = width;
		this->height = height;
		this->fullscreen = true;
		this->title = title;
		this->_lastTime = 0.0f;
	}
	
	AndroidJNI_Window::~AndroidJNI_Window()
	{
		//log("Destroying Android JNI Windowsystem");
	}

	void* AndroidJNI_Window::getIdFromBackend()
	{
		return javaVM;
	}

	void AndroidJNI_Window::enterMainLoop()
	{
		april::log("Fatal error: Using enterMainLoop on Android JNI!");
		exit(-1);
	}
	
	bool AndroidJNI_Window::updateOneFrame()
	{
		static bool result = true;
		static float t = 0.0f;
		static float k = 0.0f;
		if (this->_lastTime == 0.0f)
		{
			this->_lastTime = this->globalTimer.getTime();
		}
		this->checkEvents();
		t = this->globalTimer.getTime();
		if (t == this->_lastTime)
		{
			return true; // don't redraw frames which won't change
		}
		k = (t - this->_lastTime) / 1000.0f;
		if (k > 0.5f)
		{
			k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
		}
		this->_lastTime = t;
		result = this->performUpdate(k);
		april::rendersys->presentFrame();
		return result;
	}
	
	void AndroidJNI_Window::terminateMainLoop()
	{
		this->running = false;
	}

	void AndroidJNI_Window::destroyWindow()
	{
	}

	void AndroidJNI_Window::presentFrame()
	{
		// not needed as Android Java Activity takes care of this
	}

	void AndroidJNI_Window::checkEvents()
	{
		while (this->mouseEvents.size() > 0)
		{
			MouseInputEvent e = this->mouseEvents.pop_first();
			this->cursorPosition = e.position;
			Window::handleMouseEvent(e.type, e.position, e.button);
		}
		while (this->keyEvents.size() > 0)
		{
			KeyInputEvent e = this->keyEvents.pop_first();
			Window::handleKeyEvent(e.type, e.keyCode, e.charCode);
		}
	}

	void AndroidJNI_Window::handleMouseEvent(MouseEventType type, gvec2 position, MouseButton button)
	{
		this->mouseEvents += MouseInputEvent(type, position, button);
	}

	void AndroidJNI_Window::handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode)
	{
		this->keyEvents += KeyInputEvent(type, keyCode, charCode);
	}

	void AndroidJNI_Window::beginKeyboardHandling()
	{
		JNIEnv* env = NULL;
		jclass classInputMethodManager = NULL;
		jobject inputMethodManager = NULL;
		jobject decorView = NULL;
		_getVirtualKeyboardClasses((void**)&env, (void**)&classInputMethodManager, (void**)&inputMethodManager, (void**)&decorView);
		// show virtual keyboard
		jmethodID methodShowSoftInput = env->GetMethodID(classInputMethodManager, "showSoftInput", "(Landroid/view/View;I)Z");
		env->CallBooleanMethod(inputMethodManager, methodShowSoftInput, decorView, 0);
	}
	
	void AndroidJNI_Window::terminateKeyboardHandling()
	{
		JNIEnv* env = NULL;
		jclass classInputMethodManager = NULL;
		jobject inputMethodManager = NULL;
		jobject decorView = NULL;
		_getVirtualKeyboardClasses((void**)&env, (void**)&classInputMethodManager, (void**)&inputMethodManager, (void**)&decorView);
		// hide virtual keyboard
		jclass classView = env->FindClass("android/view/View");
		jmethodID methodGetWindowToken = env->GetMethodID(classView, "getWindowToken", "()Landroid/os/IBinder;");
		jobject binder = env->CallObjectMethod(decorView, methodGetWindowToken);
		jmethodID methodHideSoftInput = env->GetMethodID(classInputMethodManager, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
		env->CallBooleanMethod(inputMethodManager, methodHideSoftInput, binder, 0);
	}

	void AndroidJNI_Window::_getVirtualKeyboardClasses(void** javaEnv, void** javaClassInputMethodManager, void** javaInputMethodManager, void** javaDecorView)
	{
		JNIEnv* env = NULL;
		((JavaVM*)javaVM)->GetEnv((void**)&env, JNI_VERSION_1_6);
		jclass classAprilActivity = env->GetObjectClass(aprilActivity);
		jclass classContext = env->FindClass("android/content/Context");
		jfieldID fieldINPUT_METHOD_SERVICE = env->GetStaticFieldID(classContext, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
		jobject INPUT_METHOD_SERVICE = env->GetStaticObjectField(classContext, fieldINPUT_METHOD_SERVICE);
		jmethodID methodGetSystemService = env->GetMethodID(classAprilActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		jmethodID methodGetWindow = env->GetMethodID(classAprilActivity, "getWindow", "()Landroid/view/Window;");
		jobject window = env->CallObjectMethod(aprilActivity, methodGetWindow);
		jclass classWindow = env->FindClass("android/view/Window");
		jmethodID methodGetDecorView = env->GetMethodID(classWindow, "getDecorView", "()Landroid/view/View;");
		// output
		*javaEnv = env;
		*javaClassInputMethodManager = env->FindClass("android/view/inputmethod/InputMethodManager");
		*javaInputMethodManager = env->CallObjectMethod(aprilActivity, methodGetSystemService, INPUT_METHOD_SERVICE);
		*javaDecorView = env->CallObjectMethod(window, methodGetDecorView);
	}
	
}
#endif