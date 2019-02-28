/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef __ANDROID__
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hrdir.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#define __NATIVE_INTERFACE_CLASS "com/april/NativeInterface"
#include "AndroidJNI_Keys.h"
#include "AndroidJNI_Window.h"
#include "androidUtilJNI.h"
#include "Application.h"
#include "april.h"
#include "Keys.h"
#include "main_base.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "UpdateDelegate.h"
#include "Window.h"

#define PROTECTED_APPLICATION_CALL(methodCall) \
	if (april::application != NULL) \
	{ \
		april::application->methodCall; \
	}
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
#define ANDROID_WINDOW ((AndroidJNI_Window*)april::window)

namespace april
{
	extern void* javaVM;
	extern void (*dialogCallback)(MessageBoxButton);
	extern jobject classLoader;

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
		harray<hstr> args;
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
		april::application->setArgs(args);
		april::application->init();
	}

	void JNICALL _JNI_destroy(JNIEnv* env, jclass classe)
	{
		delete april::application;
		april::application = NULL;
		if (april::classLoader != NULL)
		{
			env->DeleteGlobalRef(april::classLoader);
			april::classLoader = NULL;
		}
		// nothing else may be called here, because this code is called from the Android UI thread
	}

	bool JNICALL _JNI_update(JNIEnv* env, jclass classe)
	{
		if (april::application != NULL)
		{
			// using a try-catch block here just to make sure crashes are logged properly
			try
			{
				if (april::application->getState() == april::Application::State::Starting)
				{
					april::application->updateInitializing(true);
					return true;
				}
				if (!april::application->update())
				{
					april::application->repeatLastFrame(false);
				}
			}
			catch (hexception& e)
			{
				hlog::error("FATAL", e.getFullMessage());
				throw e;
			}
			return (april::application->getState() == april::Application::State::Running);
		}
		return false;
	}

	void JNICALL _JNI_onKeyDown(JNIEnv* env, jclass classe, jint keyCode, jint charCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyInput(KeyEvent::Type::Down, android2april((int)keyCode), (unsigned int)charCode));
	}

	void JNICALL _JNI_onKeyUp(JNIEnv* env, jclass classe, jint keyCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyInput(KeyEvent::Type::Up, android2april((int)keyCode), 0));
	}

	void JNICALL _JNI_onChar(JNIEnv* env, jclass classe, jint charCode)
	{
		PROTECTED_WINDOW_CALL(queueKeyInput(KeyEvent::Type::Down, april::Key::None, (unsigned int)charCode));
	}

	void JNICALL _JNI_onTouch(JNIEnv* env, jclass classe, jint type, jint index, jfloat x, jfloat y)
	{
		PROTECTED_WINDOW_CALL(queueTouchInput(TouchEvent::Type::fromInt((int)type), (int)index, gvec2f((float)x, (float)y)));
	}

	void JNICALL _JNI_onScroll(JNIEnv* env, jclass classe, jfloat x, jfloat y)
	{
		PROTECTED_WINDOW_CALL(queueMouseInput(MouseEvent::Type::Scroll, gvec2f((float)x, (float)y), april::Key::None));
	}

	void JNICALL _JNI_onButtonDown(JNIEnv* env, jclass classe, jint controllerIndex, jint buttonCode)
	{
		PROTECTED_WINDOW_CALL(queueControllerInput(ControllerEvent::Type::Down, (int)controllerIndex, Button::fromInt((int)buttonCode), 0.0f));
	}

	void JNICALL _JNI_onButtonUp(JNIEnv* env, jclass classe, jint controllerIndex, jint buttonCode)
	{
		PROTECTED_WINDOW_CALL(queueControllerInput(ControllerEvent::Type::Up, (int)controllerIndex, Button::fromInt((int)buttonCode), 0.0f));
	}

	void JNICALL _JNI_onControllerAxisChange(JNIEnv* env, jclass classe, jint controllerIndex, jint buttonCode, jfloat axisValue)
	{
		PROTECTED_WINDOW_CALL(queueControllerInput(ControllerEvent::Type::Axis, (int)controllerIndex, Button::fromInt((int)buttonCode), axisValue));
	}

	void JNICALL _JNI_onAccelerometer(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionInput(MotionEvent::Type::Accelerometer, gvec3f(x, y, z)));
	}

	void JNICALL _JNI_onLinearAccelerometer(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionInput(MotionEvent::Type::LinearAccelerometer, gvec3f(x, y, z)));
	}

	void JNICALL _JNI_onGravity(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionInput(MotionEvent::Type::Gravity, gvec3f(x, y, z)));
	}

	void JNICALL _JNI_onRotation(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionInput(MotionEvent::Type::Rotation, gvec3f(x, y, z)));
	}

	void JNICALL _JNI_onGyroscope(JNIEnv* env, jclass classe, jfloat x, jfloat y, jfloat z)
	{
		PROTECTED_WINDOW_CALL(queueMotionInput(MotionEvent::Type::Gyroscope, gvec3f(x, y, z)));
	}

	void JNICALL _JNI_onWindowFocusChanged(JNIEnv* env, jclass classe, jboolean jFocused)
	{
#if defined(__ANDROID__) && defined(_ANDROIDJNI_WINDOW)
		if (april::window != NULL && april::window->getName() == WindowType::AndroidJNI.getName())
		{
			bool focused = (jFocused != JNI_FALSE);
			hlog::write(logTag, "onWindowFocusChanged(" + hstr(focused) + ")");
			april::window->queueFocusChange(focused);
		}
#endif
	}

	void JNICALL _JNI_onVirtualKeyboardChanged(JNIEnv* env, jclass classe, jboolean jVisible, jfloat jHeightRatio)
	{
#ifdef __ANDROID__
		if (april::window != NULL)
		{
			bool visible = (jVisible != JNI_FALSE);
			float heightRatio = (float)jHeightRatio;
			hlog::write(logTag, "onVirtualKeyboardChanged(" + hstr(visible) + ", " + hstr(heightRatio) + ")");
			april::window->queueVirtualKeyboardChange(visible, heightRatio);
		}
#endif
	}

	void JNICALL _JNI_onLowMemory(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "onLowMemoryWarning()");
		PROTECTED_WINDOW_CALL(queueLowMemoryWarning());
	}

	void JNICALL _JNI_onSurfaceCreated(JNIEnv* env, jclass classe)
	{
		if (april::rendersys != NULL)
		{
			hlog::write(logTag, "Android View::onSurfaceCreated()");
			april::rendersys->reset();
		}
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
	}

	void JNICALL _JNI_activityOnResumeNotify(JNIEnv* env, jclass classe)
	{
		PROTECTED_APPLICATION_CALL(resume());
		PROTECTED_WINDOW_CALL(queueActivityChange(true));
	}

	void JNICALL _JNI_activityOnPause(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onPause()");
		PROTECTED_RENDERSYS_CALL(suspend()); // must stay here due to threading requirement
	}
	
	void JNICALL _JNI_activityOnPauseNotify(JNIEnv* env, jclass classe)
	{
		PROTECTED_WINDOW_CALL(queueActivityChange(false));
		PROTECTED_APPLICATION_CALL(suspend());
	}

	void JNICALL _JNI_activityOnStop(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onStop()");
	}

	void JNICALL _JNI_activityOnDestroy(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onDestroy()");
		april::application->resume(); // first unlock the update thread so it can finish
		april::application->finish();
		april::application->updateFinishing();
		april::application->destroy();
	}

	void JNICALL _JNI_activityOnRestart(JNIEnv* env, jclass classe)
	{
		hlog::write(logTag, "Android Activity::onRestart()");
	}

	void JNICALL _JNI_onDialogOk(JNIEnv* env, jclass classe)
	{
		april::Application::messageBoxCallback(MessageBoxButton::Ok);
	}

	void JNICALL _JNI_onDialogYes(JNIEnv* env, jclass classe)
	{
		april::Application::messageBoxCallback(MessageBoxButton::Yes);
	}

	void JNICALL _JNI_onDialogNo(JNIEnv* env, jclass classe)
	{
		april::Application::messageBoxCallback(MessageBoxButton::No);
	}

	void JNICALL _JNI_onDialogCancel(JNIEnv* env, jclass classe)
	{
		april::Application::messageBoxCallback(MessageBoxButton::Cancel);
	}

#define METHOD_COUNT 34 // make sure this fits
	static JNINativeMethod methods[METHOD_COUNT] =
	{
		{"setVariables",						_JARGS(_JVOID, _JSTR _JSTR),					(void*)&april::_JNI_setVariables				},
		{"init",								_JARGS(_JVOID, _JARR(_JSTR)),					(void*)&april::_JNI_init						},
		{"destroy",								_JARGS(_JVOID, ),								(void*)&april::_JNI_destroy						},
		{"update",								_JARGS(_JBOOL, ),								(void*)&april::_JNI_update						},
		{"onKeyDown",							_JARGS(_JVOID, _JINT _JINT),					(bool*)&april::_JNI_onKeyDown					},
		{"onKeyUp",								_JARGS(_JVOID, _JINT),							(bool*)&april::_JNI_onKeyUp						},
		{"onChar",								_JARGS(_JVOID, _JINT),							(bool*)&april::_JNI_onChar						},
		{"onTouch",								_JARGS(_JVOID, _JINT _JINT _JFLOAT _JFLOAT),	(void*)&april::_JNI_onTouch						},
		{"onScroll",							_JARGS(_JVOID, _JFLOAT _JFLOAT),				(void*)&april::_JNI_onScroll					},
		{"onButtonDown",						_JARGS(_JVOID, _JINT _JINT),					(bool*)&april::_JNI_onButtonDown				},
		{"onButtonUp",							_JARGS(_JVOID, _JINT _JINT),					(bool*)&april::_JNI_onButtonUp					},
		{"onControllerAxisChange",				_JARGS(_JVOID, _JINT _JINT _JFLOAT),			(bool*)&april::_JNI_onControllerAxisChange		},
		{"onAccelerometer",						_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onAccelerometer				},
		{"onLinearAccelerometer",				_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onLinearAccelerometer		},
		{"onGravity",							_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onGravity					},
		{"onRotation",							_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onRotation					},
		{"onGyroscope",							_JARGS(_JVOID, _JFLOAT _JFLOAT _JFLOAT),		(void*)&april::_JNI_onGyroscope					},
		{"onWindowFocusChanged",				_JARGS(_JVOID, _JBOOL),							(void*)&april::_JNI_onWindowFocusChanged		},
		{"onVirtualKeyboardChanged",			_JARGS(_JVOID, _JBOOL _JFLOAT),					(void*)&april::_JNI_onVirtualKeyboardChanged	},
		{"onLowMemory",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onLowMemory					},
		{"onSurfaceCreated",					_JARGS(_JVOID, ),								(void*)&april::_JNI_onSurfaceCreated			},
		{"activityOnCreate",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnCreate			},
		{"activityOnStart",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStart				},
		{"activityOnResume",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnResume			},
		{"activityOnResumeNotify",				_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnResumeNotify		},
		{"activityOnPause",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnPause				},
		{"activityOnPauseNotify",				_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnPauseNotify		},
		{"activityOnStop",						_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnStop				},
		{"activityOnDestroy",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnDestroy			},
		{"activityOnRestart",					_JARGS(_JVOID, ),								(void*)&april::_JNI_activityOnRestart			},
		{"onDialogOk",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogOk					},
		{"onDialogYes",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogYes					},
		{"onDialogNo",							_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogNo					},
		{"onDialogCancel",						_JARGS(_JVOID, ),								(void*)&april::_JNI_onDialogCancel				}
	};

	jint __JNI_OnLoad(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), JavaVM* vm, void* reserved)
	{
		hlog::write(logTag, "Loading binary.");
		april::javaVM = (void*)vm;
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
		april::application = new Application(aprilApplicationInit, aprilApplicationDestroy);
		return JNI_VERSION_1_6;
	}

}
#endif
