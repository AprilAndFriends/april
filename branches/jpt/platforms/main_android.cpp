/// @file
/// @author  Boris Mikic
/// @version 1.53
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

#include "AndroidJNIWindow.h"
#include "april.h"
#include "Keys.h"
#include "main.h"
#include "RenderSystem.h"
#include "TextureManager.h"
#include "Window.h"

#define PROTECTED_RENDERSYS_GET_WINDOW(methodCall) \
	if (april::rendersys != NULL && april::window != NULL) \
	{ \
		april::window->methodCall; \
	}
#define _JSTR_TO_HSTR(string) _jstringToHstr(env, string)

namespace april
{
	extern void* javaVM;
	extern jobject jActivity;
	extern gvec2 androidResolution;
	extern void (*dialogCallback)(MessageBoxButton);

	hstr _jstringToHstr(JNIEnv* env, jstring string)
	{
		const char* chars = env->GetStringUTFChars(string, NULL);
		hstr result(chars);
		env->ReleaseStringUTFChars(string, chars);
		return result;
	}

	void JNICALL _JNI_setVariables(JNIEnv* env, jclass classe, jobject activity,
		jstring jSystemPath, jstring jSharedPath, jstring jPackageName, jstring jVersionCode, jstring jForceArchivePath)
	{
		april::jActivity = activity;
		april::systemPath = _JSTR_TO_HSTR(jSystemPath);
		hstr archivePath = _JSTR_TO_HSTR(jForceArchivePath);
		hstr packageName = _JSTR_TO_HSTR(jPackageName);
		if (!hresource::hasZip()) // if not using APK as data file archive
		{
			// set the resources CWD
			hresource::setCwd(_JSTR_TO_HSTR(jSharedPath) + "/Android/data/" + packageName);
			hresource::setArchive(""); // not used anyway when hasZip() returns false
		}
		else if (archivePath != "")
		{
			// using APK file as archive
			hresource::setCwd("assets");
			hresource::setArchive(archivePath);
		}
		else
		{
			// using OBB file as archive
			hresource::setCwd(".");
			hstr archiveName = "main." + _JSTR_TO_HSTR(jVersionCode) + "." + packageName + ".obb"; // using Google Play's "Expansion File" system
			hresource::setArchive(_JSTR_TO_HSTR(jSharedPath) + "/Android/obb/" + packageName + "/" + archiveName);
		}
	}

	void JNICALL _JNI_init(JNIEnv* env, jclass classe, jobjectArray _args, jint width, jint height)
	{
		harray<hstr> args;
		int length = env->GetArrayLength(_args);
		for_iter (i, 0, length)
		{
			args += _JSTR_TO_HSTR((jstring)env->GetObjectArrayElement(_args, i));
		}
		april::androidResolution.set((float)hmax(width, height), (float)hmin(width, height));
		april_init(args);
	}

	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		april_destroy();
	}

	bool JNICALL _JNI_render(JNIEnv* env, jclass classe)
	{
		if (april::rendersys != NULL && april::window != NULL)
		{
			return april::window->updateOneFrame();
		}
		return true;
	}

	void JNICALL _JNI_onTouch(JNIEnv* env, jclass classe, jint type, jfloat x, jfloat y, jint index)
	{
		if (april::window != NULL)
		{
			((april::AndroidJNIWindow*)april::window)->handleTouchEvent((april::Window::MouseEventType)type, (float)x, (float)y, (int)index);
		}
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

	void JNICALL _JNI_onLowMemory(JNIEnv* env, jclass classe)
	{
		PROTECTED_RENDERSYS_GET_WINDOW(handleLowMemoryWarning());
	}

	void JNICALL _JNI_onSurfaceCreated(JNIEnv* env, jclass classe)
	{
		if (april::rendersys != NULL)
		{
			april::rendersys->restore();
		}
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
		PROTECTED_RENDERSYS_GET_WINDOW(handleFocusEvent(true));
	}
	
	void JNICALL _JNI_activityOnPause(JNIEnv* env, jclass classe)
	{
#ifdef _DEBUG
		april::log("Android ActivityOnPause()");
#endif
		PROTECTED_RENDERSYS_GET_WINDOW(handleFocusEvent(false));
		if (april::rendersys != NULL)
		{
			april::rendersys->getTextureManager()->unloadTextures();
		}
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
		{"init",				_JARGS(_JVOID, _JARR(_JSTR) _JINT _JINT),				(void*)&april::_JNI_init				},
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
