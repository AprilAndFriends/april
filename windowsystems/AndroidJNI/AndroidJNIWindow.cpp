/// @file
/// @author  Boris Mikic
/// @version 1.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#include "AndroidJniWindow.h"
#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	void* javaVM = NULL;
	jobject aprilActivity = NULL;
	gvec2 androidResolution;
	static gvec2 cursorPosition;
	static april::Timer globalTimer;
	static float lastTime = 0.0f;

	AndroidJNIWindow::AndroidJNIWindow(int w, int h, bool fullscreen, chstr title) : Window()
	{
		if (april::rendersys != NULL)
		{
			april::log("Creating Android JNI Windowsystem (" + hstr(w) + ", " + hstr(h) + ")");
		}
		mWidth = w;
		mHeight = h;
		mRunning = true;
		mActive = true;
		mFullscreen = fullscreen;
		//mTouchEnabled = false;
		mTitle = title;
	}
	
	AndroidJNIWindow::~AndroidJNIWindow()
	{
		//log("Destroying Android JNI Windowsystem");
	}


	void AndroidJNIWindow::enterMainLoop()
	{
	}
	
	bool AndroidJNIWindow::updateOneFrame()
	{
		if (lastTime == 0.0f)
		{
			lastTime = globalTimer.getTime();
		}
		float t;
		doEvents();
		t = globalTimer.getTime();
		if (t == lastTime)
		{
			return true; // don't redraw frames which won't change
		}
		float k = (t - lastTime) / 1000.0f;
		if (k > 0.5f)
		{
			k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
		}

		lastTime = t;
		if (!mActive)
		{
			k = 0;
			for_iter (i, 0, 5)
			{
				mMouseEvents.clear();
				mKeyEvents.clear();
				hthread::sleep(40);
			}
		}
		bool result = true;
		if (mUpdateCallback != NULL)
		{
			result = (*mUpdateCallback)(k);
		}
		april::rendersys->presentFrame();
		return result;
	}
	
	void AndroidJNIWindow::terminateMainLoop()
	{
		mRunning = false;
	}

	void AndroidJNIWindow::destroyWindow()
	{
	}

	void AndroidJNIWindow::showSystemCursor(bool visible)
	{
	}

	bool AndroidJNIWindow::isSystemCursorShown()
	{
		return false;
	}

	int AndroidJNIWindow::getWidth()
	{
		return mWidth;
	}

	int AndroidJNIWindow::getHeight()
	{
		return mHeight;
	}

	void AndroidJNIWindow::setWindowTitle(chstr title)
	{
	}

	gvec2 AndroidJNIWindow::getCursorPosition()
	{
		return cursorPosition;
	}

	void AndroidJNIWindow::presentFrame()
	{
		// not needed as Android Java Activity takes care of this
	}

	void* AndroidJNIWindow::getIDFromBackend()
	{
		return javaVM;
	}

	void AndroidJNIWindow::doEvents()
	{
		while (mMouseEvents.size() > 0)
		{
			MouseInputEvent e = mMouseEvents.pop_first();
			cursorPosition.set(e.x, e.y);
			Window::handleMouseEvent(e.type, e.x, e.y, e.button);
		}
		while (mKeyEvents.size() > 0)
		{
			KeyInputEvent e = mKeyEvents.pop_first();
			Window::handleKeyEvent(e.type, e.keyCode, e.charCode);
		}
	}

	void AndroidJNIWindow::_getVirtualKeyboardClasses(void** javaEnv, void** javaClassInputMethodManager, void** javaInputMethodManager, void** javaDecorView)
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
	
	void AndroidJNIWindow::beginKeyboardHandling()
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
	
	void AndroidJNIWindow::terminateKeyboardHandling()
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

	void AndroidJNIWindow::handleMouseEvent(MouseEventType type, float x, float y, MouseButton button)
	{
		mMouseEvents += MouseInputEvent(type, x, y, button);
	}

	void AndroidJNIWindow::handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode)
	{
		mKeyEvents += KeyInputEvent(type, keyCode, charCode);
	}

	Window::DeviceType AndroidJNIWindow::getDeviceType()
	{
		return Window::DEVICE_ANDROID_PHONE;
	}

}
#endif