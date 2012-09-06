/// @file
/// @author  Boris Mikic
/// @version 2.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "AndroidJNI_Window.h"
#include "april.h"
#include "Keys.h"
#include "main.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

#define PROTECTED_WINDOW_CALL(methodCall) \
	if (april::window != NULL) \
	{ \
		april::window->methodCall; \
	}
#define PROTECTED_RENDERSYS_CALL(methodCall) \
	if (april::rendersys != NULL) \
	{ \
		april::rendersys->methodCall; \
	}
#define _JSTR_TO_HSTR(string) _jstringToHstr(env, string)

namespace april
{
	extern void* javaVM;
	JNIEnv* getJNIEnv();
	jobject getActivity();
	extern void (*dialogCallback)(MessageBoxButton);

	hstr _jstringToHstr(JNIEnv* env, jstring string)
	{
		const char* chars = env->GetStringUTFChars(string, NULL);
		hstr result(chars);
		env->ReleaseStringUTFChars(string, chars);
		return result;
	}

	void JNICALL _JNI_setVariables(JNIEnv* env, jclass classe, jobject activity,
		jstring jSystemPath, jstring jDataPath, jstring jPackageName, jstring jVersionCode, jstring jForceArchivePath)
	{
		april::systemPath = _JSTR_TO_HSTR(jSystemPath);
		hstr archivePath = _JSTR_TO_HSTR(jForceArchivePath);
		hstr packageName = _JSTR_TO_HSTR(jPackageName);
#ifdef _DEBUG
		april::log("system path: " + april::systemPath);
#endif
		if (!hresource::hasZip()) // if not using APK as data file archive
		{
			// set the resources CWD
			hresource::setCwd(_JSTR_TO_HSTR(jDataPath) + "/Android/data/" + packageName);
			hresource::setArchive(""); // not used anyway when hasZip() returns false
#ifdef _DEBUG
			april::log("using no-zip: " + hresource::getCwd());
#endif
		}
		else if (archivePath != "")
		{
			// using APK file as archive
			hresource::setCwd("assets");
			hresource::setArchive(archivePath);
#ifdef _DEBUG
			april::log("using assets: " + hresource::getArchive());
#endif
		}
		else
		{
			// using OBB file as archive
			hresource::setCwd(".");
			// using Google Play's "Expansion File" system
			hresource::setArchive(_JSTR_TO_HSTR(jDataPath) + "/main." + _JSTR_TO_HSTR(jVersionCode) + "." + packageName + ".obb");
#ifdef _DEBUG
			april::log("using obb: " + hresource::getArchive());
#endif
		}
	}

	void JNICALL _JNI_init(JNIEnv* env, jclass classe, jobjectArray _args)
	{
		harray<hstr> args;
		int length = env->GetArrayLength(_args);
		for_iter (i, 0, length)
		{
			args += _JSTR_TO_HSTR((jstring)env->GetObjectArrayElement(_args, i));
		}
		april_init(args);
	}

	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		april_destroy();
	}

	bool JNICALL _JNI_render(JNIEnv* env, jclass classe)
	{
		if (april::window != NULL)
		{
			return april::window->updateOneFrame();
		}
		return true;
	}

	void JNICALL _JNI_onTouch(JNIEnv* env, jclass classe, jint type, jfloat x, jfloat y, jint index)
	{
		if (april::window != NULL)
		{
			((april::AndroidJNI_Window*)april::window)->handleTouchEvent((april::Window::MouseEventType)type, gvec2((float)x, (float)y), (int)index);
		}
	}

	bool JNICALL _JNI_onKeyDown(JNIEnv* env, jclass classe, jint keyCode, jint charCode)
	{
		PROTECTED_WINDOW_CALL(handleKeyEvent(april::Window::AKEYEVT_DOWN, (KeySym)(int)keyCode, (unsigned int)charCode));
		return true;
	}

	bool JNICALL _JNI_onKeyUp(JNIEnv* env, jclass classe, jint keyCode)
	{
		PROTECTED_WINDOW_CALL(handleKeyEvent(april::Window::AKEYEVT_UP, (KeySym)(int)keyCode, 0));
		return true;
	}

	void JNICALL _JNI_onLowMemory(JNIEnv* env, jclass classe)
	{
		PROTECTED_WINDOW_CALL(handleLowMemoryWarning());
	}

	void JNICALL _JNI_onSurfaceCreated(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android View::onSurfaceCreated()");
#endif
		PROTECTED_RENDERSYS_CALL(reset());
	}

	void JNICALL _JNI_activityOnCreate(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android Activity::onCreate()");
#endif
	}

	void JNICALL _JNI_activityOnStart(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android Activity::onStart()");
#endif
	}

	void JNICALL _JNI_activityOnResume(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android Activity::onResume()");
#endif
		PROTECTED_WINDOW_CALL(handleFocusChangeEvent(true));
	}
	
	void JNICALL _JNI_activityOnPause(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android Activity::onPause()");
#endif
		PROTECTED_WINDOW_CALL(handleFocusChangeEvent(false));
		PROTECTED_RENDERSYS_CALL(unloadTextures());
	}

	void JNICALL _JNI_activityOnStop(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android Activity::onStop()");
#endif
	}

	void JNICALL _JNI_activityOnDestroy(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android Activity::onDestroy()");
#endif
	}

	void JNICALL _JNI_activityOnRestart(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android Activity::onRestart()");
#endif
	}

	void JNICALL _JNI_onDialogOk(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(AMSGBTN_OK);
		}
	}

	void JNICALL _JNI_onDialogYes(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(AMSGBTN_YES);
		}
	}

	void JNICALL _JNI_onDialogNo(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(AMSGBTN_NO);
		}
	}

	void JNICALL _JNI_onDialogCancel(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(AMSGBTN_CANCEL);
		}
	}

#define _JARGS(returnType, arguments) "(" arguments ")" returnType
#define _JARR(str) "[" str
#define _JOBJ "Ljava/lang/Object;"
#define _JSTR "Ljava/lang/String;"
#define _JINT "I"
#define _JBOOL "Z"
#define _JFLOAT "F"
#define _JVOID "V"

#define METHOD_COUNT 20 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"setVariables",		_JARGS(_JVOID, _JOBJ _JSTR _JSTR _JSTR _JSTR _JSTR),	(void*)&april::_JNI_setVariables		},
		{"init",				_JARGS(_JVOID, _JARR(_JSTR)),							(void*)&april::_JNI_init				},
		{"destroy",				_JARGS(_JVOID, ),										(void*)&april::_JNI_destroy				},
		{"render",				_JARGS(_JBOOL, ),										(void*)&april::_JNI_render				},
		{"onTouch",				_JARGS(_JVOID, _JINT _JFLOAT _JFLOAT _JINT),			(void*)&april::_JNI_onTouch				},
		{"onKeyDown",			_JARGS(_JBOOL, _JINT _JINT),							(bool*)&april::_JNI_onKeyDown			},
		{"onKeyUp",				_JARGS(_JBOOL, _JINT),									(bool*)&april::_JNI_onKeyUp				},
		{"onLowMemory",			_JARGS(_JVOID, ),										(void*)&april::_JNI_onLowMemory			},
		{"onSurfaceCreated",	_JARGS(_JVOID, ),										(void*)&april::_JNI_onSurfaceCreated	},
		{"activityOnCreate",	_JARGS(_JVOID, ),										(void*)&april::_JNI_activityOnCreate	},
		{"activityOnStart",		_JARGS(_JVOID, ),										(void*)&april::_JNI_activityOnStart		},
		{"activityOnResume",	_JARGS(_JVOID, ),										(void*)&april::_JNI_activityOnResume	},
		{"activityOnPause",		_JARGS(_JVOID, ),										(void*)&april::_JNI_activityOnPause		},
		{"activityOnStop",		_JARGS(_JVOID, ),										(void*)&april::_JNI_activityOnStop		},
		{"activityOnDestroy",	_JARGS(_JVOID, ),										(void*)&april::_JNI_activityOnDestroy	},
		{"activityOnRestart",	_JARGS(_JVOID, ),										(void*)&april::_JNI_activityOnRestart	},
		{"onDialogOk",			_JARGS(_JVOID, ),										(void*)&april::_JNI_onDialogOk			},
		{"onDialogYes",			_JARGS(_JVOID, ),										(void*)&april::_JNI_onDialogYes			},
		{"onDialogNo",			_JARGS(_JVOID, ),										(void*)&april::_JNI_onDialogNo			},
		{"onDialogCancel",		_JARGS(_JVOID, ),										(void*)&april::_JNI_onDialogCancel		}
	};
	
	jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		april::javaVM = (void*)vm;
		JNIEnv* env = april::getJNIEnv();
		if (env == NULL)
		{
			return -1;
		}
		jclass classe = env->FindClass("net/sourceforge/april/android/NativeInterface");
		if (env->RegisterNatives(classe, methods, METHOD_COUNT) != 0)
		{
			return -1;
		}
		return JNI_VERSION_1_6;
	}

}

#endif
