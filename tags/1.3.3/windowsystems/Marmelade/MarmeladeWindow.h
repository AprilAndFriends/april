/// @file
/// @author  Domagoj Cerjan
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a Marmalade window.

#ifndef APRIL_MARMALADE_WINDOW_H
#define APRIL_MARMALADE_WINDOW_H

#ifdef HAVE_MARMELADE

#include <s3e.h>
#include <GLES/gl.h>
#include <GLES/egl.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Window.h"
#include "harray.h"

namespace april
{
	enum InputEventType
	{
		AIET_KEYBOARD_EVENT = 0,
		AIET_MOUSE_EVENT,
		AIET_MOUSE_MOTION_EVENT,
		AIET_TOUCH_EVENT,
		AIET_TOUCH_MOTION_EVENT,
		AIET_UNKNOWN
	};

	enum InputEventState
	{
		AIES_STATE_UP = 0,
		AIES_STATE_DOWN,
		AIES_STATE_PRESSED,
		AIES_STATE_RELEASED,
		AIES_STATE_UNKNOWN
	};

	class InputEvent
	{
	public:
		InputEvent(InputEventType type = AIET_UNKNOWN);

		InputEventType mType;
	};

	class InputKeyboardEvent : public InputEvent
	{
	public:
		InputKeyboardEvent(KeySym key = AK_UNKNOWN, InputEventState state = AIES_STATE_UNKNOWN);

		KeySym mSym;
		InputEventState mState;
	};

	class InputMouseEvent : public InputEvent
	{
	public:
		InputMouseEvent(KeySym button = AK_UNKNOWN, InputEventState state = AIES_STATE_UNKNOWN, int32 x = 0, int32 y = 0);

		KeySym mSym;
		InputEventState mState;
		int32 mX, mY;
	};

	class InputMouseMotionEvent : public InputEvent
	{
	public:
		InputMouseMotionEvent(int32 x = 0, int32 y = 0);

		int32 mX, mY;
	};

	class InputTouchEvent : public InputEvent
	{
	public:
		InputTouchEvent(int32 id = 0, InputEventState state = AIES_STATE_UNKNOWN, int32 x = 0, int32 y = 0);

		int32 mID;
		InputEventState mState;
		int32 mX, mY;
	};

	class InputTouchMotionEvent : public InputEvent
	{
	public:
		InputTouchMotionEvent(int32 id = -1, int32 x = 0, int32 y = 0);

		int32 mID, mX, mY;
	};

	class MarmeladeWindow : public Window
	{
	public:
		MarmeladeWindow(int w, int h, bool fullscreen, chstr title);
		~MarmeladeWindow();
		
		// implementations
		void enterMainLoop();
		void terminateMainLoop();
		void destroyWindow();
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWidth();
		int getHeight();
		void setWindowTitle(chstr title);
		gvec2 getCursorPosition();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		bool isCursorInside();

		/* 
		 * these callbacks are used only to translate marmelade/s3e events to ours
		 * standard april window callback policies still apply
		 */
		static int32 keyboardHandler(void *sys, void *args);
		static int32 mouseClickHandler(void *sys, void *args);
		static int32 mouseMotionHandler(void *sys, void *args);
		static int32 touchTapHandler(void *sys, void *args);
		static int32 touchMotionHandler(void *sys, void *args);
		static int32 pauseHandler(void *sys, void *args);
		static int32 unpauseHandler(void *sys, void *args);
		static void update();

		DeviceType getDeviceType();

		float mCursorX; // TODO turn into private
		float mCursorY; // TODO turn into private

		void pushEvent(InputEvent *evt);
		void setRunning(bool running);
		
	private:
		
		uint16 *mScreen;
		bool mCursorVisible;
		bool mCursorInside;

		bool _eglInit();
		void _eglTerminate();

		EGLSurface g_EGLSurface;
		EGLDisplay g_EGLDisplay;
		EGLDisplay g_EGLContext;

		harray<InputEvent*> mEvents;

	};
}

#endif
#endif
