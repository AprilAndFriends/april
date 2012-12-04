/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.5
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
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Keys.h"
#include "Timer.h"

namespace april
{
	class RenderSystem;

	class aprilExport Window
	{
	public:
		enum MouseEventType
		{
			AMOUSEEVT_DOWN = 0,
			AMOUSEEVT_UP = 1,
			AMOUSEEVT_MOVE = 2,
			AMOUSEEVT_SCROLL = 3
		};
		
		enum KeyEventType
		{
			AKEYEVT_DOWN = 0,
			AKEYEVT_UP = 1
		};
		
		enum MouseButton
		{
			AMOUSEBTN_NONE = 0,
			AMOUSEBTN_LEFT = 1,
			AMOUSEBTN_RIGHT = 2,
			AMOUSEBTN_MIDDLE = 3,
			AMOUSEBTN_WHEELUP = 4,
			AMOUSEBTN_WHEELDOWN = 5,
			AMOUSEBTN_DOUBLETAP = 7
		};

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
		
		Window();
		virtual ~Window();
		virtual bool create(int w, int h, bool fullscreen, chstr title);
		virtual bool destroy();

		// generic getters/setters
		hstr getName() { return this->name; }
		bool isCreated() { return this->created; }
		hstr getTitle() { return this->title; }
		bool isFullscreen() { return this->fullscreen; }
		void setFullscreen(bool value) { } // TODO
		bool isFocused() { return this->focused; }
		bool isRunning() { return this->running; }
		int getFps() { return this->fps; }
		float getFpsResolution() { return this->fpsResolution; }
		void setFpsResolution(float value) { this->fpsResolution = value; }
		gvec2 getCursorPosition() { return this->cursorPosition; }
		gvec2 getSize();
		float getAspectRatio();

		// callbacks
		typedef bool (*updateFunction)(float);
		void setUpdateCallback(updateFunction func) { this->updateCallback = func; }
		updateFunction getUpdateCallback() { return this->updateCallback; }
		// TODO - update this
		void setMouseDownCallback(void (*value)(int)) { this->mouseDownCallback = value; }
		void setMouseUpCallback(void (*value)(int)) { this->mouseUpCallback = value; }
		void setMouseMoveCallback(void (*value)()) { this->mouseMoveCallback = value; }
		void setMouseScrollCallback(void (*value)(float, float)) { this->mouseScrollCallback = value; }
		void setKeyDownCallback(void (*value)(unsigned int)) { this->keyDownCallback = value; }
		void setKeyUpCallback(void (*value)(unsigned int)) { this->keyUpCallback = value; }
		void setCharCallback(void (*value)(unsigned int)) { this->charCallback = value; }
		void setQuitCallback(bool (*value)(bool)) { this->quitCallback = value; }
		void setFocusChangeCallback(void (*value)(bool)) { this->focusChangeCallback = value; }
		void setTouchscreenEnabledCallback(void (*value)(bool)) { this->touchscreenEnabledCallback = value; }
		void setTouchEventCallback(void (*value)(harray<gvec2>&)) { this->touchCallback = value; }
		void setDeviceOrientationCallback(void (*value)(DeviceOrientation)) { this->deviceOrientationCallback = value; }
		void setVirtualKeyboardCallback(void (*value)(bool)) { this->virtualKeyboardCallback = value; }
		void setHandleUrlCallback(bool (*value)(chstr)) { this->handleUrlCallback = value; }
		void setLowMemoryCallback(void (*value)()) { this->lowMemoryCallback = value; }

		// virtual getters/setters
		virtual void setTitle(chstr value) { this->title = value; }
		virtual bool isVirtualKeyboardVisible() { return false; }
		virtual bool isCursorVisible() { return this->cursorVisible; }
		virtual void setCursorVisible(bool value) { this->cursorVisible = value; }
		virtual bool isCursorInside();

		virtual void _setResolution(int w, int h) { }
		
		// pure virtual getters/setters (window system dependent)
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;
		virtual void setTouchEnabled(bool value) = 0;
		virtual bool isTouchEnabled() = 0;
		virtual void* getBackendId() = 0;

		// additional callback setters
		void setMouseCallbacks(void (*mouseDownCallback)(int),
							   void (*mouseUpCallback)(int),
							   void (*mouseMoveCallback)(),
							   void (*mouseScrollCallback)(float, float));
		
		void getMouseCallbacks(void (**mouseDownCallback)(int),
							   void (**mouseUpCallback)(int),
							   void (**mouseMoveCallback)(),
							   void (**mouseScrollCallback)(float, float));
		
		void setKeyboardCallbacks(void (*keyDownCallback)(unsigned int),
								  void (*keyUpCallback)(unsigned int),
								  void (*charCallback)(unsigned int));
		
		void getKeyboardCallbacks(void (**keyDownCallback)(unsigned int),
								  void (**keyUpCallback)(unsigned int),
								  void (**charCallback)(unsigned int));


		// pure virtual methods (window system dependent)
		virtual void presentFrame() = 0;
		virtual void checkEvents() = 0;
		virtual bool updateOneFrame();
		virtual void terminateMainLoop();

		// misc virtuals
		virtual void beginKeyboardHandling() { }
		virtual void terminateKeyboardHandling() { }
		
		virtual bool isRotating() { return false; } // iOS/Android devices for example
		virtual hstr getParam(chstr param) { return ""; }
		virtual void setParam(chstr param, chstr value) { }
		
		// generic but overridable event handlers
		virtual void handleMouseEvent(MouseEventType type, gvec2 position, MouseButton button);
		virtual void handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode);
		virtual bool handleQuitRequest(bool canReject);
		virtual void handleFocusChangeEvent(bool focused);
		virtual void handleTouchscreenEnabledEvent(bool enabled);
		virtual void handleTouchEvent(harray<gvec2>& touches);
		virtual bool handleUrl(chstr url);
		virtual void handleLowMemoryWarning();

		void handleKeyOnlyEvent(KeyEventType type, KeySym keyCode);
		void handleCharOnlyEvent(unsigned int charCode);

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
		bool focused;
		bool running;
		int fps;
		int fpsCount;
		float fpsTimer;
		float fpsResolution;
		gvec2 cursorPosition;
		bool cursorVisible;
		Timer timer;

		// TODO - refactor
		static void (*msLaunchCallback)(void*);

		bool (*updateCallback)(float);
		void (*mouseDownCallback)(int);
		void (*mouseUpCallback)(int);
		void (*mouseMoveCallback)();
		void (*mouseScrollCallback)(float, float);
		void (*keyDownCallback)(unsigned int);
		void (*keyUpCallback)(unsigned int);
		void (*charCallback)(unsigned int);
		bool (*quitCallback)(bool can_reject);
		void (*focusChangeCallback)(bool);
		void (*touchscreenEnabledCallback)(bool);
		void (*touchCallback)(harray<gvec2>&);
		void (*deviceOrientationCallback)(DeviceOrientation);
		void (*virtualKeyboardCallback)(bool);
		bool (*handleUrlCallback)(chstr);
		void (*lowMemoryCallback)();

		virtual float _calcTimeSinceLastFrame();

	};

	// global window shortcut variable
	aprilFnExport extern april::Window* window;

}
#endif
