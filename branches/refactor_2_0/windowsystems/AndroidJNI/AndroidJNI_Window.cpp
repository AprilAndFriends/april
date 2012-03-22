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

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#include "AndroidJNI_Window.h"
#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"

namespace april
{
	void* javaVM = NULL;
	jobject jActivity = NULL;
	gvec2 androidResolution;

	AndroidJNI_Window::AndroidJNI_Window() : Window(), width(0), height(0), alreadyTouched(false), _lastTime(0.0f)
	{
		this->name = "Android JNI";
	}

	AndroidJNI_Window::~AndroidJNI_Window()
	{
		this->destroy();
	}

	bool AndroidJNI_Window::create(int width, int height, bool fullscreen, chstr title)
	{
		if (!Window::create(width, height, true, title))
		{
			return false;
		}
		this->width = width;
		this->height = height;
		this->alreadyTouched = false;
		this->_lastTime = 0.0f;
		return true;
	}
	
	void* AndroidJNI_Window::getBackendId()
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
		while (this->touchEvents.size() > 0)
		{
			TouchInputEvent e = this->touchEvents.pop_first();
			Window::handleTouchEvent(e.touches);
		}
	}

	void AndroidJNI_Window::handleMouseEvent(MouseEventType type, gvec2 position, MouseButton button)
	{
		/* // TODO - potential multitouch handling
#ifdef _DEBUG
		april::log("    - MOUSE EVENT -> " + hstr(type == AMOUSEEVT_UP ? "UP" : (type == AMOUSEEVT_DOWN ? "DOWN" : "MOV")));
#endif
		if (type == AMOUSEEVT_DOWN && mAlreadyTouched)
		{
#ifdef _DEBUG
			april::log("    - TOUCH " + hstr(position.x) + " " + hstr(position.y));
#endif
			this->alreadyTouched = false;
			this->mouseEvents += MouseInputEvent(AMOUSEEVT_UP, position, button);
			harray<gvec2> touches;
			touches += cursorPosition;
			touches += position;
			this->touchEvents += TouchInputEvent(touches);
		}
		else if (type == AMOUSEEVT_DOWN)
		{
#ifdef _DEBUG
			april::log("    - DOWN " + hstr(position.x) + " " + hstr(position.y));
#endif
			this->alreadyTouched = true;
			this->mouseEvents += MouseInputEvent(type, position, button);
		}
		else if (type == AMOUSEEVT_MOVE)
		{
#ifdef _DEBUG
			april::log("    - MOV " + hstr(position.x) + " " + hstr(position.y));
#endif
			this->mouseEvents += MouseInputEvent(type, position, button);
		}
		else if (type == AMOUSEEVT_UP)
		{
			if (this->alreadyTouched)
			{
				this->aAlreadyTouched = false;
				this->mouseEvents += MouseInputEvent(type, position, button);
#ifdef _DEBUG
				april::log("    - UP " + hstr(position.x) + " " + hstr(position.y));
#endif
			}
		}
		//*/
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
		this->_getVirtualKeyboardClasses((void**)&env, (void**)&classInputMethodManager, (void**)&inputMethodManager, (void**)&decorView);
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
		this->_getVirtualKeyboardClasses((void**)&env, (void**)&classInputMethodManager, (void**)&inputMethodManager, (void**)&decorView);
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
		jclass classAprilActivity = env->GetObjectClass(jActivity);
		jclass classContext = env->FindClass("android/content/Context");
		jfieldID fieldINPUT_METHOD_SERVICE = env->GetStaticFieldID(classContext, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
		jobject INPUT_METHOD_SERVICE = env->GetStaticObjectField(classContext, fieldINPUT_METHOD_SERVICE);
		jmethodID methodGetSystemService = env->GetMethodID(classAprilActivity, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		jmethodID methodGetWindow = env->GetMethodID(classAprilActivity, "getWindow", "()Landroid/view/Window;");
		jobject window = env->CallObjectMethod(jActivity, methodGetWindow);
		jclass classWindow = env->FindClass("android/view/Window");
		jmethodID methodGetDecorView = env->GetMethodID(classWindow, "getDecorView", "()Landroid/view/View;");
		// output
		*javaEnv = env;
		*javaClassInputMethodManager = env->FindClass("android/view/inputmethod/InputMethodManager");
		*javaInputMethodManager = env->CallObjectMethod(jActivity, methodGetSystemService, INPUT_METHOD_SERVICE);
		*javaDecorView = env->CallObjectMethod(window, methodGetDecorView);
	}

}
#endif