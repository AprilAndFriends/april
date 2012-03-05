/// @file
/// @author  Boris Mikic
/// @version 1.51
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "Keys.h"
#include "main.h"
#include "RenderSystem.h"
#include "TextureManager.h"
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
	extern gvec2 androidResolution;

	void JNICALL _JNI_init(JNIEnv* env, jclass classe, jobject activity, jobjectArray _args, jstring path, jint width, jint height)
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
		if (!hresource::hasZip()) // if not using APK as data file archive
		{
			// set the resources CWD
			hstr packageName = get_basedir(april::systemPath).split("/", -1, true).pop_last();
			hresource::setCwd("/mnt/sdcard/Android/data/" + packageName); // I am so going to hell for this one
		}
		else
		{
			// using current APK file as archive
			hresource::setArchive(args[0]);
		}
		april::androidResolution.set((float)hmax((int)width, (int)height), (float)hmin((int)width, (int)height));
		april_init(args);
	}

	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		april_destroy();
	}

	bool JNICALL _JNI_render(JNIEnv* env, jclass classe)
	{
		if (april::rendersys != NULL && april::rendersys->getWindow() != NULL)
		{
			return april::rendersys->getWindow()->updateOneFrame();
		}
		return true;
	}

	bool JNICALL _JNI_onQuit(JNIEnv* env, jclass classe)
	{
		if (april::rendersys != NULL && april::rendersys->getWindow() != NULL)
		{
			return april::rendersys->getWindow()->handleQuitRequest(true);
		}
		return true;
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

	void JNICALL _JNI_activityOnCreate(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnCreate()");
#endif
	}

	void JNICALL _JNI_activityOnStart(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnStart()");
#endif
	}

	void JNICALL _JNI_activityOnResume(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnResume()");
#endif
	}

	void JNICALL _JNI_activityOnPause(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnPause()");
#endif
	}

	void JNICALL _JNI_activityOnStop(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnStop()");
#endif
	}

	void JNICALL _JNI_activityOnDestroy(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnDestroy()");
#endif
	}

	void JNICALL _JNI_activityOnRestart(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnRestart()");
#endif
		if (april::rendersys != NULL)
		{
			april::rendersys->getTextureManager()->unloadTextures();
		}
	}

#define METHOD_COUNT 18 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"init",				"(Ljava/lang/Object;[Ljava/lang/String;Ljava/lang/String;II)V",	(void*)&april::_JNI_init				},
		{"destroy",				"()V",															(void*)&april::_JNI_destroy				},
		{"render",				"()Z",															(void*)&april::_JNI_render				},
		{"onQuit",				"()Z",															(void*)&april::_JNI_onQuit				},
		{"onMouseDown",			"(FFI)V",														(void*)&april::_JNI_onMouseDown			},
		{"onMouseUp",			"(FFI)V",														(void*)&april::_JNI_onMouseUp			},
		{"onMouseMove",			"(FF)V",														(void*)&april::_JNI_onMouseMove			},
		{"onKeyDown",			"(II)Z",														(bool*)&april::_JNI_onKeyDown			},
		{"onKeyUp",				"(I)Z",															(bool*)&april::_JNI_onKeyUp				},
		{"onFocusChange",		"(Z)V",															(void*)&april::_JNI_onFocusChange		},
		{"onLowMemory",			"()V",															(void*)&april::_JNI_onLowMemory			},
		{"activityOnCreate",	"()V",															(void*)&april::_JNI_activityOnCreate	},
		{"activityOnStart",		"()V",															(void*)&april::_JNI_activityOnStart		},
		{"activityOnResume",	"()V",															(void*)&april::_JNI_activityOnResume	},
		{"activityOnPause",		"()V",															(void*)&april::_JNI_activityOnPause		},
		{"activityOnStop",		"()V",															(void*)&april::_JNI_activityOnStop		},
		{"activityOnDestroy",	"()V",															(void*)&april::_JNI_activityOnDestroy	},
		{"activityOnRestart",	"()V",															(void*)&april::_JNI_activityOnRestart	},
	};
	
	jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		april::javaVM = (void*)vm;
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
