/// @file
/// @author  Boris Mikic
/// @version 1.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <android/log.h>
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "Keys.h"
#include "main.h"
#include "RenderSystem.h"
#include "Window.h"

#define PROTECTED_RENDERSYS_GET_WINDOW(methodCall) \
	if (april::rendersys != NULL && april::rendersys->getWindow() != NULL) \
	{ \
		april::rendersys->getWindow()->methodCall; \
	}

namespace april
{
	extern void* javaVM;
	extern jobject aprilActivity;

	void JNICALL _JNI_init(JNIEnv* env, jclass classe, jobject activity, jobjectArray _args, jstring path)
	{
		aprilActivity = activity;
		harray<hstr> args;
		jstring arg;
		const char* str;
		int length = env->GetArrayLength(_args);
		for (int i = 0; i < length; i++)
		{
			arg = (jstring)env->GetObjectArrayElement(_args, i);
			str = env->GetStringUTFChars(arg, NULL);
			args += hstr(str);
			env->ReleaseStringUTFChars(arg, str);
		}
		str = env->GetStringUTFChars(path, NULL);
		april::systemPath = hstr(str);
		env->ReleaseStringUTFChars(path, str);
		hresource::setArchive(args[0]);
		april_init(args);
	}

	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		april_destroy();
	}

	void JNICALL _JNI_render(JNIEnv* env, jclass classe)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(updateOneFrame());
	}

	void JNICALL _JNI_onMouseDown(JNIEnv* env, jclass classe, jfloat x, jfloat y, jint button)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleMouseEvent(april::Window::AMOUSEEVT_DOWN, (float)x, (float)y, april::Window::AMOUSEBTN_LEFT));
	}

	void JNICALL _JNI_onMouseUp(JNIEnv* env, jclass classe, jfloat x, jfloat y, jint button)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleMouseEvent(april::Window::AMOUSEEVT_UP, (float)x, (float)y, april::Window::AMOUSEBTN_LEFT));
	}

	void JNICALL _JNI_onMouseMove(JNIEnv* env, jclass classe, jfloat x, jfloat y)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleMouseEvent(april::Window::AMOUSEEVT_MOVE, (float)x, (float)y, april::Window::AMOUSEBTN_LEFT));
	}
	
	bool JNICALL _JNI_onKeyDown(JNIEnv* env, jclass classe, jint keyCode, jint charCode)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleKeyEvent(april::Window::AKEYEVT_DOWN, (KeySym)(int)keyCode, (unsigned int)charCode));
		return true;
	}

	bool JNICALL _JNI_onKeyUp(JNIEnv* env, jclass classe, jint keyCode)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleKeyEvent(april::Window::AKEYEVT_UP, (KeySym)(int)keyCode, 0));
		return true;
	}

	void JNICALL _JNI_onFocusChange(JNIEnv* env, jclass classe, jboolean has_focus)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleFocusEvent((bool)has_focus));
	}

	void JNICALL _JNI_onLowMemory(JNIEnv* env, jclass classe)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleLowMemoryWarning());
	}

#define METHOD_COUNT 10 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"init",			"(Ljava/lang/Object;[Ljava/lang/String;Ljava/lang/String;)V",	(void*)&april::_JNI_init			},
		{"destroy",			"()V",															(void*)&april::_JNI_destroy			},
		{"render",			"()V",															(void*)&april::_JNI_render			},
		{"onMouseDown",		"(FFI)V",														(void*)&april::_JNI_onMouseDown		},
		{"onMouseUp",		"(FFI)V",														(void*)&april::_JNI_onMouseUp		},
		{"onMouseMove",		"(FF)V",														(void*)&april::_JNI_onMouseMove		},
		{"onKeyDown",		"(II)Z",														(bool*)&april::_JNI_onKeyDown		},
		{"onKeyUp",			"(I)Z",															(bool*)&april::_JNI_onKeyUp			},
		{"onFocusChange",	"(Z)V",															(void*)&april::_JNI_onFocusChange	},
		{"onLowMemory",		"()V",															(void*)&april::_JNI_onLowMemory		}
	};
	
	jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		JNIEnv* env;
		april::javaVM = (void*)vm;
		if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
		{
			return -1;
		}
		jclass classe = env->FindClass("net/sourceforge/april/AprilJNI");
		if (env->RegisterNatives(classe, methods, METHOD_COUNT) != 0)
		{
			return -1;
		}
		return JNI_VERSION_1_6;
	}
}

#endif
