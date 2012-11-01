/// @file
/// @author  Boris Mikic
/// @version 2.42
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
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

namespace april
{
	extern void* javaVM;
	extern void (*dialogCallback)(MessageBoxButton);
	
	hstr _jstringToHstr(JNIEnv* env, jstring string)
	{
		const char* chars = env->GetStringUTFChars(string, NULL);
		hstr result(chars);
		env->ReleaseStringUTFChars(string, chars);
		return result;
	}
	
	void JNICALL _JNI_setVariables(JNIEnv* env, jclass classe, jstring jSystemPath,
		jstring jDataPath, jstring jPackageName, jstring jVersionCode, jstring jForceArchivePath)
	{
		april::systemPath = _JSTR_TO_HSTR(jSystemPath);
		hstr archivePath = _JSTR_TO_HSTR(jForceArchivePath);
		hstr packageName = _JSTR_TO_HSTR(jPackageName);
		hlog::debug(april::logTag, "System path: " + april::systemPath);
		if (!hresource::hasZip()) // if not using APK as data file archive
		{
			// set the resources CWD
			hresource::setCwd(_JSTR_TO_HSTR(jDataPath) + "/Android/data/" + packageName);
			hresource::setArchive(""); // not used anyway when hasZip() returns false
			hlog::debug(april::logTag, "Using no-zip: " + hresource::getCwd());
		}
		else if (archivePath != "")
		{
			// using APK file as archive
			hresource::setCwd("assets");
			hresource::setArchive(archivePath);
			hlog::debug(april::logTag, "Using assets: " + hresource::getArchive());
		}
		else
		{
			// using OBB file as archive
			hresource::setCwd(".");
			// using Google Play's "Expansion File" system
			hresource::setArchive(_JSTR_TO_HSTR(jDataPath) + "/main." + _JSTR_TO_HSTR(jVersionCode) + "." + packageName + ".obb");
			hlog::debug(april::logTag, "Using obb: " + hresource::getArchive());
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
		hlog::debug(april::logTag, "Got args: " + args.join(","));
		foreach (hstr, it, args)
		{
			hlog::debug(april::logTag, "  " + (*it));
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
	
	void JNICALL _JNI_onWindowFocusChanged(JNIEnv* env, jclass classe, jboolean focused)
	{
		if (focused != JNI_FALSE)
		{
			// only TRUE is propagated as FALSE is already handled by onPause()
			PROTECTED_WINDOW_CALL(handleFocusChangeEvent(true));
		}
	}
	
	void JNICALL _JNI_onLowMemory(JNIEnv* env, jclass classe)
	{
		PROTECTED_WINDOW_CALL(handleLowMemoryWarning());
	}
	
	void JNICALL _JNI_onSurfaceCreated(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android View::onSurfaceCreated()");
		PROTECTED_RENDERSYS_CALL(reset());
	}
	
	void JNICALL _JNI_activityOnCreate(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android Activity::onCreate()");
	}
	
	void JNICALL _JNI_activityOnStart(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android Activity::onStart()");
	}
	
	void JNICALL _JNI_activityOnResume(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android Activity::onResume()");
	}
	
	void JNICALL _JNI_activityOnPause(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android Activity::onPause()");
		// has to be here because of a problem on certain devices where audio volume change window takes away focus
		PROTECTED_WINDOW_CALL(handleFocusChangeEvent(false));
		PROTECTED_RENDERSYS_CALL(unloadTextures());
	}
	
	void JNICALL _JNI_activityOnStop(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android Activity::onStop()");
	}
	
	void JNICALL _JNI_activityOnDestroy(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android Activity::onDestroy()");
	}
	
	void JNICALL _JNI_activityOnRestart(JNIEnv* env, jclass classe)
	{
		hlog::debug(april::logTag, "Android Activity::onRestart()");
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
	
#define METHOD_COUNT 21 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"setVariables",			_JARGS(_JVOID, _JSTR _JSTR _JSTR _JSTR _JSTR),	(void*)&april::_JNI_setVariables			},
		{"init",					_JARGS(_JVOID, _JARR(_JSTR)),					(void*)&april::_JNI_init					},
		{"destroy",					_JARGS(_JVOID, ),								(void*)&april::_JNI_destroy					},
		{"render",					_JARGS(_JBOOL, ),								(void*)&april::_JNI_render					},
		{"onTouch",					_JARGS(_JVOID, _JINT _JFLOAT _JFLOAT _JINT),	(void*)&april::_JNI_onTouch					},
		{"onKeyDown",				_JARGS(_JBOOL, _JINT _JINT),					(bool*)&april::_JNI_onKeyDown				},
		{"onKeyUp",					_JARGS(_JBOOL, _JINT),							(bool*)&april::_JNI_onKeyUp					},
		{"onWindowFocusChanged",	_JARGS(_JVOID, _JBOOL),							(void*)&april::_JNI_onWindowFocusChanged	},
		{"onLowMemory",				_JARGS(_JVOID, ),								(void*)&april::_JNI_onLowMemory				},
		{"onSurfaceCreated",		_JARGS(_JVOID, ),								(void*)&april::_JNI_onSurfaceCreated		},
		{"activityOnCreate",		_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnCreate		},
		{"activityOnStart",			_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStart			},
		{"activityOnResume",		_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnResume		},
		{"activityOnPause",			_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnPause			},
		{"activityOnStop",			_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStop			},
		{"activityOnDestroy",		_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnDestroy		},
		{"activityOnRestart",		_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnRestart		},
		{"onDialogOk",				_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogOk				},
		{"onDialogYes",				_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogYes				},
		{"onDialogNo",				_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogNo				},
		{"onDialogCancel",			_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogCancel			}
	};
	
	jint JNI_OnLoad(JavaVM* vm, void* reserved)
	{
		april::javaVM = (void*)vm;
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		if (env->RegisterNatives(classNativeInterface, methods, METHOD_COUNT) != 0)
		{
			return -1;
		}
		return JNI_VERSION_1_6;
	}

}

#endif
