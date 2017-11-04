/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _ANDROID
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#define __NATIVE_INTERFACE_CLASS "com/april/NativeInterface"
#include "androidUtilJNI.h"
#include "april.h"
#include "Keys.h"
#include "main_base.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "AndroidJNI_Keys.h"

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
	void (*aprilInit)(const harray<hstr>&) = NULL;
	void (*aprilDestroy)() = NULL;
	extern jobject classLoader;
	extern harray<hstr> args;
	
	void JNICALL _JNI_setVariables(JNIEnv* env, jclass classe, jstring jDataPath, jstring jForcedArchivePath)
	{
		hstr dataPath = _JSTR_TO_HSTR(jDataPath);
		hstr archivePath = _JSTR_TO_HSTR(jForcedArchivePath);
		hlog::write(logTag, "System path: " + april::getUserDataPath());
		if (!hresource::hasZip()) // if not using APK as data file archive
		{
			// using user data directory for resources
			harray<hstr> segments;
			segments += dataPath;
			segments += "Android/data";
			segments += april::getPackageName();
			hresource::mountArchive("", hrdir::joinPaths(segments));
			hlog::write(logTag, "Using user data directory for resources.");
		}
		else if (archivePath != "")
		{
			// using APK file's "assets" directory for resources
			hresource::mountArchive("", archivePath, "assets");
			hlog::write(logTag, "Using assets for resources: " + archivePath);
		}
		else
		{
			// using Google Play's "Expansion File" system for resources
			hresource::mountArchive("", dataPath);
			hlog::write(logTag, "Using obb for resources: " + dataPath);
		}
	}
	
	void JNICALL _JNI_init(JNIEnv* env, jclass classe, jobjectArray jArgs)
	{
		int length = env->GetArrayLength(jArgs);
		jstring string = NULL;
		for_iter (i, 0, length)
		{
			string = (jstring)env->GetObjectArrayElement(jArgs, i);
			args += _JSTR_TO_HSTR(string);
			env->DeleteLocalRef(string);
		}
		hlog::debug(logTag, "Got args:");
		foreach (hstr, it, args)
		{
			hlog::debug(logTag, "    " + (*it));
		}
		(*aprilInit)(args);
	}
	
	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		(*aprilDestroy)();
	}
	
	bool JNICALL _JNI_render(JNIEnv* env, jclass classe)
	{
		bool result = true;
		if (april::window != NULL)
		{
			try
			{
				result = april::window->updateOneFrame();
				if (april::rendersys != NULL)
				{
					april::rendersys->flushFrame(true);
				}
			}
			catch (hexception& e)
			{
				hlog::error("FATAL", e.getFullMessage());
				throw e;
			}
		}
		return result;
	}
	
	void JNICALL _JNI_onKeyDown(JNIEnv* env, jclass classe, jint keyCode, jint charCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyEvent(april::Window::KeyInputEvent::Type::Down, android2april((int)keyCode), (unsigned int)charCode));
	}
	
	void JNICALL _JNI_onKeyUp(JNIEnv* env, jclass classe, jint keyCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyEvent(april::Window::KeyInputEvent::Type::Up, android2april((int)keyCode), 0));
	}
	
	void JNICALL _JNI_onChar(JNIEnv* env, jclass classe, jint charCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyEvent(april::Window::KeyInputEvent::Type::Down, april::Key::None, (unsigned int)charCode));
	}

	void JNICALL _JNI_onTouch(JNIEnv* env, jclass classe, jint type, jfloat x, jfloat y, jint index)
	{
		PROTECTED_WINDOW_CALL(queueTouchEvent(april::Window::MouseInputEvent::Type::Type::fromInt((int)type), gvec2((float)x, (float)y), (int)index));
	}
	
	void JNICALL _JNI_onButtonDown(JNIEnv* env, jclass classe, jint controllerIndex, jint buttonCode)
	{
		PROTECTED_WINDOW_CALL(queueControllerEvent(april::Window::ControllerInputEvent::Type::Down, (int)controllerIndex, Button::fromInt((int)buttonCode), 0.0f));
	}
	
	void JNICALL _JNI_onButtonUp(JNIEnv* env, jclass classe, jint controllerIndex, jint buttonCode)
	{
		PROTECTED_WINDOW_CALL(queueControllerEvent(april::Window::ControllerInputEvent::Type::Up, (int)controllerIndex, Button::fromInt((int)buttonCode), 0.0f));
	}
	
	void JNICALL _JNI_onControllerAxisChange(JNIEnv* env, jclass classe, jint controllerIndex, jint buttonCode, jfloat axisValue)
	{
		PROTECTED_WINDOW_CALL(queueControllerEvent(april::Window::ControllerInputEvent::Type::Axis, (int)controllerIndex, Button::fromInt((int)buttonCode), axisValue));
	}

	void JNICALL _JNI_onGravity(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionEvent(april::Window::MotionInputEvent::Type::Gravity, gvec3(x, y, z)));
	}

	void JNICALL _JNI_onLinearAccelerometer(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionEvent(april::Window::MotionInputEvent::Type::LinearAccelerometer, gvec3(x, y, z)));
	}

	void JNICALL _JNI_onRotation(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionEvent(april::Window::MotionInputEvent::Type::Rotation, gvec3(x, y, z)));
	}

	void JNICALL _JNI_onGyroscope(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionEvent(april::Window::MotionInputEvent::Type::Gyroscope, gvec3(x, y, z)));
	}

	void JNICALL _JNI_onWindowFocusChanged(JNIEnv* env, jclass classe, jboolean jFocused)
	{
		bool focused = (jFocused != JNI_FALSE);
		hlog::write(logTag, "onWindowFocusChanged(" + hstr(focused) + ")");
		PROTECTED_WINDOW_CALL(handleFocusChangeEvent(focused));
	}
	
	void JNICALL _JNI_onVirtualKeyboardChanged(JNIEnv* env, jclass classe, jboolean jVisible, jfloat jHeightRatio)
	{
		bool visible = (jVisible != JNI_FALSE);
		float heightRatio = (float)jHeightRatio;
		hlog::write(logTag, "onVirtualKeyboardChanged(" + hstr(visible) + "," + hstr(heightRatio) + ")");
		PROTECTED_WINDOW_CALL(handleVirtualKeyboardChangeEvent(visible, heightRatio));
	}
	
	void JNICALL _JNI_onLowMemory(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "onLowMemoryWarning()");
		PROTECTED_WINDOW_CALL(handleLowMemoryWarningEvent());
	}
	
	void JNICALL _JNI_onSurfaceCreated(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android View::onSurfaceCreated()");
		PROTECTED_RENDERSYS_CALL(reset());
	}
	
	void JNICALL _JNI_activityOnCreate(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onCreate()");
	}
	
	void JNICALL _JNI_activityOnStart(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onStart()");
	}
	
	void JNICALL _JNI_activityOnResume(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onResume()");
		PROTECTED_WINDOW_CALL(handleActivityChange(true));
	}
	
	void JNICALL _JNI_activityOnPause(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onPause()");
		PROTECTED_WINDOW_CALL(handleActivityChange(false));
		PROTECTED_RENDERSYS_CALL(suspend());
	}
	
	void JNICALL _JNI_activityOnStop(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onStop()");
	}
	
	void JNICALL _JNI_activityOnDestroy(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onDestroy()");
		if (april::classLoader != NULL)
		{
			env->DeleteGlobalRef(april::classLoader);
			april::classLoader = NULL;
		}
	}
	
	void JNICALL _JNI_activityOnRestart(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onRestart()");
	}
	
	void JNICALL _JNI_onDialogOk(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MessageBoxButton::Ok);
		}
	}
	
	void JNICALL _JNI_onDialogYes(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MessageBoxButton::Yes);
		}
	}
	
	void JNICALL _JNI_onDialogNo(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MessageBoxButton::No);
		}
	}
	
	void JNICALL _JNI_onDialogCancel(JNIEnv* env, jclass classe)
	{
		if (dialogCallback != NULL)
		{
			(*dialogCallback)(MessageBoxButton::Cancel);
		}
	}
	
#define METHOD_COUNT 30 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"setVariables",						_JARGS(_JVOID, _JSTR _JSTR),					(void*)&april::_JNI_setVariables				},
		{"init",								_JARGS(_JVOID, _JARR(_JSTR)),					(void*)&april::_JNI_init						},
		{"destroy",								_JARGS(_JVOID, ),								(void*)&april::_JNI_destroy						},
		{"render",								_JARGS(_JBOOL, ),								(void*)&april::_JNI_render						},
		{"onKeyDown",							_JARGS(_JVOID, _JINT _JINT),					(bool*)&april::_JNI_onKeyDown					},
		{"onKeyUp",								_JARGS(_JVOID, _JINT),							(bool*)&april::_JNI_onKeyUp						},
		{"onChar",								_JARGS(_JVOID, _JINT),							(bool*)&april::_JNI_onChar						},
		{"onTouch",								_JARGS(_JVOID, _JINT _JFLOAT _JFLOAT _JINT),	(void*)&april::_JNI_onTouch						},
		{"onButtonDown",						_JARGS(_JVOID, _JINT _JINT),					(bool*)&april::_JNI_onButtonDown				},
		{"onButtonUp",							_JARGS(_JVOID, _JINT _JINT),					(bool*)&april::_JNI_onButtonUp					},
		{"onControllerAxisChange",				_JARGS(_JVOID, _JINT _JINT _JFLOAT),			(bool*)&april::_JNI_onControllerAxisChange		},
		{"onGravity",							_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onGravity					},
		{"onLinearAccelerometer",				_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onLinearAccelerometer		},
		{"onRotation",							_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onRotation					},
		{"onGyroscope",							_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onGyroscope					},
		{"onWindowFocusChanged",				_JARGS(_JVOID, _JBOOL),							(void*)&april::_JNI_onWindowFocusChanged		},
		{"onVirtualKeyboardChanged",			_JARGS(_JVOID, _JBOOL _JFLOAT),					(void*)&april::_JNI_onVirtualKeyboardChanged	},
		{"onLowMemory",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onLowMemory					},
		{"onSurfaceCreated",					_JARGS(_JVOID, ),								(void*)&april::_JNI_onSurfaceCreated			},
		{"activityOnCreate",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnCreate			},
		{"activityOnStart",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStart				},
		{"activityOnResume",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnResume			},
		{"activityOnPause",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnPause				},
		{"activityOnStop",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStop				},
		{"activityOnDestroy",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnDestroy			},
		{"activityOnRestart",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnRestart			},
		{"onDialogOk",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogOk					},
		{"onDialogYes",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogYes					},
		{"onDialogNo",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogNo					},
		{"onDialogCancel",						_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogCancel				}
	};


	
	jint __JNI_OnLoad(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), JavaVM* vm, void* reserved)
	{
		april::javaVM = (void*)vm;
		april::aprilInit = anAprilInit;
		april::aprilDestroy = anAprilDestroy;
		JNIEnv* env = NULL;
		if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
		{
			return -1;
		}
		jclass classNativeInterface = env->FindClass(__NATIVE_INTERFACE_CLASS);
		if (env->RegisterNatives(classNativeInterface, methods, METHOD_COUNT) != 0)
		{
			return -1;
		}
		jclass classClass = env->FindClass("java/lang/Class");
		jmethodID methodGetClassLoader = env->GetMethodID(classClass, "getClassLoader", _JARGS(_JCLASS("java/lang/ClassLoader"), ));
		jobject classLoader = env->CallObjectMethod(classNativeInterface, methodGetClassLoader);
		april::classLoader = env->NewGlobalRef(classLoader);
		return JNI_VERSION_1_6;
	}

}
#endif
