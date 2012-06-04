/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
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

namespace april
{
	class RenderSystem;

	class aprilExport Window
	{
		static void (*msLaunchCallback)();
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
		
		virtual ~Window();

		// utility funcs
		void _platformCursorVisibilityUpdate(bool visible);

		// generic getters/setters
		hstr getTitle() { return this->title; }
		bool isFullscreen() { return this->fullscreen; }
		bool isFocused() { return this->focused; }
		bool isRunning() { return this->running; }
		gvec2 getCursorPosition() { return this->cursorPosition; }
		bool isCursorVisible() { return this->cursorVisible; }
		gvec2 getSize();
		float getAspectRatio();
		bool isCursorInside();

		// callback setters
		void setUpdateCallback(bool (*value)(float)) { this->updateCallback = value; }
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
		virtual bool isVirtualKeyboardVisible() { return false; }
		virtual void setCursorVisible(bool value) { this->cursorVisible = value; }
		virtual void setTitle(chstr value) { this->title = value; }

		virtual void _setResolution(int width, int height) { }
		
		// pure virtual getters/setters (window system dependent)
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;
		virtual void setTouchEnabled(bool value) = 0;
		virtual bool isTouchEnabled() = 0;
		virtual void* getIDFromBackend() = 0;

		// additional callback setters
		void setMouseCallbacks(void (*mouseDownCallback)(int),
							   void (*mouseUpCallback)(int),
							   void (*mouseMoveCallback)(),
							   void (*mouseScrollCallback)(float, float));
		void setKeyboardCallbacks(void (*keyDownCallback)(unsigned int),
								  void (*keyUpCallback)(unsigned int),
								  void (*charCallback)(unsigned int));

		// pure virtual methods (window system dependent)
		virtual void enterMainLoop() = 0;
		virtual bool updateOneFrame() = 0;
		virtual void terminateMainLoop() = 0;
		virtual void destroyWindow() = 0;
		virtual void presentFrame() = 0;
		virtual void doEvents() = 0;

		// misc virtuals
		virtual void beginKeyboardHandling() { }
		virtual void terminateKeyboardHandling() { }
		virtual float prefixRotationAngle() { return 0.0f; }
		
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
		virtual void handleLaunchCallback(); // TODO - this may have to be refactored

		virtual bool performUpdate(float k);
		
		DEPRECATED_ATTRIBUTE hstr getWindowTitle() { return this->getTitle(); }
		DEPRECATED_ATTRIBUTE virtual void setWindowTitle(chstr value) { this->setTitle(value); }
		DEPRECATED_ATTRIBUTE void setWindowFocusCallback(void (*value)(bool)) { this->setFocusChangeCallback(value); }
		DEPRECATED_ATTRIBUTE bool handleURL(chstr url) { return this->handleUrl(url); }
		DEPRECATED_ATTRIBUTE void setHandleURLCallback(bool (*value)(chstr)) { this->setHandleUrlCallback(value); }
		DEPRECATED_ATTRIBUTE gvec2 getDimensions() { return this->getSize(); }
		DEPRECATED_ATTRIBUTE bool isKeyboardVisible() { return this->isVirtualKeyboardVisible(); }
		DEPRECATED_ATTRIBUTE void showSystemCursor(bool value) { this->setCursorVisible(value); }
		DEPRECATED_ATTRIBUTE bool isSystemCursorShown() { return this->isCursorVisible(); }
		DEPRECATED_ATTRIBUTE void handleFocusEvent(bool focused) { this->handleFocusChangeEvent(focused); }

		DEPRECATED_ATTRIBUTE void setMouseCallbacks(void (*_mouseDownCallback)(float, float, int),
													void (*_mouseUpCallback)(float, float, int),
													void (*_mouseMoveCallback)(float, float),
													void (*_mouseScrollCallback)(float, float) = NULL);

	protected:
		Window();
		
		hstr title;
		bool fullscreen;
		bool focused;
		bool running;
		gvec2 cursorPosition;
		bool cursorVisible;

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

		void _handleKeyOnlyEvent(KeyEventType type, KeySym keyCode);
		void _handleCharOnlyEvent(unsigned int charCode);

		// DEPRECATED
		void (*mouseDownCallback_DEPRECATED)(float, float, int);
		void (*mouseUpCallback_DEPRECATED)(float, float, int);
		void (*mouseMoveCallback_DEPRECATED)(float, float);
	};


	// global window shortcut variable
	aprilFnExport extern april::Window* window;

}
#endif
