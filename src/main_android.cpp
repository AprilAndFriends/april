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

#include "main.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	hstr packageName;

	void JNICALL _JNI_init(JNIEnv* env, jclass classe, jobjectArray _args)
	{
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
		hresource::setArchive(args[0]);
		april_init(args);
	}

	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		april_destroy();
	}

	void JNICALL _JNI_render(JNIEnv* env, jclass classe)
	{
		april::rendersys->getWindow()->updateOneFrame();
	}

	void JNICALL _JNI_onMouseDown(JNIEnv* env, jclass classe, jfloat x, jfloat y, jint button)
	{
		april::rendersys->getWindow()->handleMouseEvent(april::Window::AMOUSEEVT_DOWN, (float)x, (float)y, april::Window::AMOUSEBTN_LEFT);
	}

	void JNICALL _JNI_onMouseUp(JNIEnv* env, jclass classe, jfloat x, jfloat y, jint button)
	{
		april::rendersys->getWindow()->handleMouseEvent(april::Window::AMOUSEEVT_UP, (float)x, (float)y, april::Window::AMOUSEBTN_LEFT);
	}

	void JNICALL _JNI_onMouseMove(JNIEnv* env, jclass classe, jfloat x, jfloat y)
	{
		april::rendersys->getWindow()->handleMouseEvent(april::Window::AMOUSEEVT_MOVE, (float)x, (float)y, april::Window::AMOUSEBTN_LEFT);
	}

#define METHOD_COUNT 6 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"init",		"([Ljava/lang/String;)V",	(void*)&april::_JNI_init		},
		{"destroy",		"()V",						(void*)&april::_JNI_destroy		},
		{"render",		"()V",						(void*)&april::_JNI_render		},
		{"onMouseDown",	"(FFI)V",					(void*)&april::_JNI_onMouseDown	},
		{"onMouseUp",	"(FFI)V",					(void*)&april::_JNI_onMouseUp	},
		{"onMouseMove",	"(FF)V",					(void*)&april::_JNI_onMouseMove	}
	};
	
	jint JNI_OnLoad(JavaVM* vm, void* reserved, chstr packageName)
	{
		april::packageName = packageName;
		JNIEnv* env;
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
