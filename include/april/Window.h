/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic window.

#ifndef APRIL_WINDOW_H
#define APRIL_WINDOW_H

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Keys.h"
#include "Timer.h"

namespace april
{
	class ControllerDelegate;
	class KeyboardDelegate;
	class MouseDelegate;
	class RenderSystem;
	class SystemDelegate;
	class TouchDelegate;
	class UpdateDelegate;

	class aprilExport Window
	{
	public:
		enum MouseEventType
		{
			AMOUSEEVT_DOWN = 0,
			AMOUSEEVT_UP = 1,
			AMOUSEEVT_CANCEL = 2, // canceling a down event
			AMOUSEEVT_MOVE = 3,
			AMOUSEEVT_SCROLL = 4
		};
		
		enum KeyEventType
		{
			AKEYEVT_DOWN = 0,
			AKEYEVT_UP = 1
		};

		enum ControllerEventType
		{
			ACTRLEVT_DOWN = 0,
			ACTRLEVT_UP = 1
			// TODO - possibly add analog triggers
		};

		enum InputMode
		{
			MOUSE,
			TOUCH,
			CONTROLLER
		};
		
		// TODOa - remove?
		enum DeviceOrientation
		{
			ADEVICEORIENTATION_NONE = 0,
			ADEVICEORIENTATION_PORTRAIT,
			ADEVICEORIENTATION_PORTRAIT_UPSIDEDOWN,
			ADEVICEORIENTATION_LANDSCAPE_LEFT, // bottom of device is on the left
			ADEVICEORIENTATION_LANDSCAPE_RIGHT, // bottom of device is on the right
			ADEVICEORIENTATION_FACE_DOWN, // screen is facing the ground
			ADEVICEORIENTATION_FACE_UP // screen is facing the sky
		};
		
		struct KeyInputEvent
		{
			KeyEventType type;
			Key keyCode;
			unsigned int charCode;
		
			KeyInputEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		
		};

		struct MouseInputEvent
		{
			MouseEventType type;
			gvec2 position;
			Key keyCode;
		
			MouseInputEvent(MouseEventType type, gvec2 position, Key keyCode);
		
		};

		struct TouchInputEvent
		{
			harray<gvec2> touches;
		
			TouchInputEvent(harray<gvec2>& touches);
		
		};

		struct ControllerInputEvent
		{
			ControllerEventType type;
			Button buttonCode;
		
			ControllerInputEvent(ControllerEventType type, Button buttonCode);
		
		};

		struct aprilExport Options
		{
		public:
			bool resizable;
			bool fpsCounter;

			Options();
			~Options();

			hstr toString();

		};

		Window();
		virtual ~Window();
		virtual bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		virtual bool destroy();
		virtual void unassign();

		// generic getters/setters
		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(Options, options, Options);
		HL_DEFINE_IS(created, Created);
		HL_DEFINE_GET(hstr, title, Title);
		HL_DEFINE_IS(fullscreen, Fullscreen);
		void setFullscreen(bool value);
		HL_DEFINE_IS(focused, Focused);
		HL_DEFINE_IS(running, Running);
		HL_DEFINE_GETSET(int, fps, Fps);
		HL_DEFINE_GETSET(float, fpsResolution, FpsResolution);
		HL_DEFINE_GET(gvec2, cursorPosition, CursorPosition);
		HL_DEFINE_IS(virtualKeyboardVisible, VirtualKeyboardVisible);
		HL_DEFINE_GET(float, virtualKeyboardHeightRatio, VirtualKeyboardHeightRatio);
		HL_DEFINE_GET(InputMode, inputMode, InputMode);
		HL_DEFINE_GET2(hmap, InputMode, InputMode, inputModeTranslations, InputModeTranslations);
		void setInputModeTranslations(hmap<InputMode, InputMode> value);
		void setInputMode(InputMode value);
		gvec2 getSize();
		float getAspectRatio();
		
		HL_DEFINE_GETSET2(hmap, Key, Button, controllerEmulationKeys, ControllerEmulationKeys);

		// callbacks
		HL_DEFINE_GETSET(UpdateDelegate*, updateDelegate, UpdateDelegate);
		HL_DEFINE_GETSET(KeyboardDelegate*, keyboardDelegate, KeyboardDelegate);
		HL_DEFINE_GETSET(MouseDelegate*, mouseDelegate, MouseDelegate);
		HL_DEFINE_GETSET(TouchDelegate*, touchDelegate, TouchDelegate);
		HL_DEFINE_GETSET(ControllerDelegate*, controllerDelegate, ControllerDelegate);
		HL_DEFINE_GETSET(SystemDelegate*, systemDelegate, SystemDelegate);

		// virtual getters/setters
		virtual void setTitle(chstr value) { this->title = value; }
		virtual bool isCursorVisible() { return this->cursorVisible; }
		virtual void setCursorVisible(bool value) { this->cursorVisible = value; }
		virtual bool isCursorInside();

		virtual void setResolution(int w, int h);
		virtual void setResolution(int w, int h, bool fullscreen);
		
		// pure virtual getters/setters (window system dependent)
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;
		DEPRECATED_ATTRIBUTE bool isTouchEnabled() { return (this->inputMode == TOUCH); }
		virtual void* getBackendId() = 0;

		// pure virtual methods (window system dependent)
		virtual void presentFrame() = 0;

		// misc virtuals
		virtual bool updateOneFrame();
		virtual void checkEvents();
		virtual void terminateMainLoop();
		virtual void beginKeyboardHandling() { }
		virtual void terminateKeyboardHandling() { }
		
		virtual bool isRotating() { return false; } // iOS/Android devices for example
		virtual hstr getParam(chstr param) { return ""; } // TODOa - this should be refactored
		virtual void setParam(chstr param, chstr value) { } // TODOa - this should be refactored
		
		// generic but overridable event handlers
		virtual void handleMouseEvent(MouseEventType type, gvec2 position, Key keyCode);
		virtual void handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		virtual void handleTouchEvent(const harray<gvec2>& touches);
		virtual void handleControllerEvent(ControllerEventType type, Button buttonCode);
		virtual bool handleQuitRequest(bool canCancel);
		virtual void handleFocusChangeEvent(bool focused);
		virtual void handleActivityChangeEvent(bool active);
		virtual void handleVirtualKeyboardChangeEvent(bool visible, float heightRatio);
		virtual void handleLowMemoryWarning();

		void handleKeyOnlyEvent(KeyEventType type, Key keyCode);
		void handleCharOnlyEvent(unsigned int charCode);

		virtual void queueKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		virtual void queueMouseEvent(MouseEventType type, gvec2 position, Key keyCode);
		virtual void queueTouchEvent(MouseEventType type, gvec2 position, int index);
		virtual void queueControllerEvent(ControllerEventType type, Button buttonCode);

		virtual void enterMainLoop();
		virtual bool performUpdate(float k);
		
		// TODO - refactor
		// the following functions should be temporary, it was added because I needed access to
		// iOS early initialization process. When april will be refactored this needs to be changed --kspes
		static void setLaunchCallback(void (*callback)(void*)) { msLaunchCallback = callback; }
		static void handleLaunchCallback(void* args);

	protected:
		bool created;
		hstr name;
		hstr title;
		bool fullscreen;
		Options options;
		bool focused;
		bool running;
		int fps;
		int fpsCount;
		float fpsTimer;
		float fpsResolution;
		gvec2 cursorPosition;
		bool cursorVisible;
		bool virtualKeyboardVisible;
		float virtualKeyboardHeightRatio;
		InputMode inputMode;
		hmap<InputMode, InputMode> inputModeTranslations;
		bool multiTouchActive;
		harray<gvec2> touches;
		harray<KeyInputEvent> keyEvents;
		harray<MouseInputEvent> mouseEvents;
		harray<TouchInputEvent> touchEvents;
		harray<ControllerInputEvent> controllerEvents;
		Timer timer;
		hmap<Key, Button> controllerEmulationKeys;

		// TODO - refactor
		static void (*msLaunchCallback)(void*);

		UpdateDelegate* updateDelegate;
		KeyboardDelegate* keyboardDelegate;
		MouseDelegate* mouseDelegate;
		TouchDelegate* touchDelegate;
		ControllerDelegate* controllerDelegate;
		SystemDelegate* systemDelegate;

		virtual float _calcTimeSinceLastFrame();
		void _setRenderSystemResolution();
		virtual void _setRenderSystemResolution(int w, int h, bool fullscreen);

	};

	// global window shortcut variable
	aprilFnExport extern april::Window* window;

}
#endif
