/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 1.82
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
#include <hltypes/hstring.h>
#include <hltypes/hmap.h>

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	class RenderSystem;

	struct SystemInfo
	{
		hstr name;
		int cpu_cores; // number of CPU cores or separate CPU units
		int ram; //! how many MB's of ram does the host system have in total
		int max_texture_size;
		hstr locale; //! current system locale code
	};
	
	class aprilExport Window
	{
	protected:
		static Window* mSingleton;
		static void (*msLaunchCallback)();
	public:
		// data types
		static Window* getSingleton() { return mSingleton; };
		virtual ~Window();

		enum DeviceType
		{
			DEVICE_IPHONE = 0,
			DEVICE_IPAD,
			DEVICE_ANDROID_PHONE,
			DEVICE_ANDROID_TABLET,
			DEVICE_WINDOWS_PC,
			DEVICE_LINUX_PC,
			DEVICE_MAC_PC,
			DEVICE_WINDOWS_PHONE,
			DEVICE_WINDOWS_TABLET,
			DEVICE_UNKNOWN_LARGE,
			DEVICE_UNKNOWN_MEDIUM,
			DEVICE_UNKNOWN_SMALL,
			DEVICE_UNKNOWN
		};

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
			AMOUSEBTN_LEFT,
			AMOUSEBTN_RIGHT,
			AMOUSEBTN_MIDDLE,
			AMOUSEBTN_WHEELUP,
			AMOUSEBTN_WHEELDN
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
		
		// utility funcs
		void _platformCursorVisibilityUpdate(bool visible);
		
		// the following functions should be temporary, it was added because I needed access to
		// iOS early initialization process. When april will be refactored this needs to be changed --kspes
		static void setLaunchCallback(void (*callback)()) { msLaunchCallback = callback; }
		static void handleLaunchCallback();
		
		
		void setUpdateCallback(bool (*callback)(float));
		DEPRECATED_ATTRIBUTE void setMouseCallbacks(void (*mouse_dn)(float, float, int),
							   void (*mouse_up)(float, float, int),
							   void (*mouse_move)(float, float),
							   void (*mouse_scroll)(float, float) = NULL);
		void setMouseCallbacks(void (*mouse_dn)(int),
							   void (*mouse_up)(int),
							   void (*mouse_move)(),
							   void (*mouse_scroll)(float, float));
		void setKeyboardCallbacks(void (*key_dn)(unsigned int),
								  void (*key_up)(unsigned int),
								  void (*char_callback)(unsigned int));
		void setQuitCallback(bool (*quit_callback)(bool can_reject));
		void setWindowFocusCallback(void (*focus_callback)(bool));
		void setVirtualKeyboardCallback(void (*vk_callback)(bool));
		void setTouchscreenEnabledCallback(void (*te_callback)(bool));
		void setTouchEventCallback(void (*t_callback)(harray<gvec2>&));
		void setLowMemoryCallback(void (*lowmem_callback)());
		void setHandleURLCallback(bool (*url_callback)(chstr));
		virtual void setDeviceOrientationCallback(void (*vk_callback)(DeviceOrientation)); 
		virtual void _setResolution(int w, int h) { }
		
		// misc pure virtuals
		virtual void enterMainLoop() = 0;
		virtual bool updateOneFrame() = 0;
		virtual void terminateMainLoop() = 0;
		virtual void destroyWindow() = 0;
		virtual void showSystemCursor(bool b) = 0;
		virtual bool isSystemCursorShown() = 0;
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;
		gvec2 getDimensions();
		DEPRECATED_ATTRIBUTE int getWindowWidth() { return getWidth(); }
		DEPRECATED_ATTRIBUTE int getWindowHeight() { return getHeight(); }
		virtual void setWindowTitle(chstr title) = 0;
		hstr getWindowTitle() { return mTitle; }
		virtual gvec2 getCursorPosition() = 0;
		virtual bool isCursorInside();
		DEPRECATED_ATTRIBUTE gvec2 getCursorPos() { return getCursorPosition(); }
		virtual void presentFrame() = 0;
		virtual void* getIDFromBackend() = 0;
		virtual void doEvents() = 0;
		virtual DeviceType getDeviceType() = 0;
		float getAspectRatio();
		DEPRECATED_ATTRIBUTE float getWindowAspectRatio() { return getAspectRatio(); }
		
		// misc virtuals
		virtual bool isFullscreen();
		virtual void beginKeyboardHandling();
		virtual void terminateKeyboardHandling();
		virtual bool isKeyboardVisible();
		virtual float prefixRotationAngle();
		
		virtual bool isRotating() { return false; } // iOS/Android devices for example
		virtual hstr getParam(chstr param) { return ""; }
		virtual void setParam(chstr param, chstr value) { }
		
		// generic but overridable event handlers
		virtual void handleMouseEvent(MouseEventType type, float x, float y, MouseButton button);
		void handleTouchEvent(harray<gvec2>& touches);
		virtual void handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode);
		virtual bool handleQuitRequest(bool can_reject);
		virtual void handleFocusEvent(bool has_focus);
		
		bool handleURL(chstr url);
		virtual bool performUpdate(float time_increase);
		void handleLowMemoryWarning();
		
	protected:
		Window();
		
		DeviceType mDeviceType;
		hstr mTitle;
		bool mFullscreen;
		
		void (*mLowMemoryCallback)();
		bool (*mUpdateCallback)(float);
		void (*mMouseDownCallback)(int);
		void (*mMouseUpCallback)(int);
		void (*mMouseMoveCallback)();
		void (*mMouseScrollCallback)(float, float);
		void (*mKeyDownCallback)(unsigned int);
		void (*mKeyUpCallback)(unsigned int);
		void (*mCharCallback)(unsigned int);
		bool (*mQuitCallback)(bool can_reject);
		void (*mFocusCallback)(bool);
		void (*mVKeyboardCallback)(bool);
		void (*mTouchEnabledCallback)(bool);
		void (*mTouchCallback)(harray<gvec2>&);
		void (*mDeviceOrientationCallback)(DeviceOrientation);
		bool (*mHandleURLCallback)(chstr);

		// DEPRECATED
		void (*mMouseDownCallback_DEPRECATED)(float, float, int);
		void (*mMouseUpCallback_DEPRECATED)(float, float, int);
		void (*mMouseMoveCallback_DEPRECATED)(float, float);

		bool mRunning;
		
	};
	
	enum MessageBoxButton
	{
		AMSGBTN_NULL = 0,
		AMSGBTN_OK = 1,
		AMSGBTN_CANCEL = 2,
		AMSGBTN_YES = 4,
		AMSGBTN_NO = 8,
		
		AMSGBTN_OKCANCEL = AMSGBTN_OK | AMSGBTN_CANCEL,
		AMSGBTN_YESNO = AMSGBTN_YES | AMSGBTN_NO,
		AMSGBTN_YESNOCANCEL = AMSGBTN_YESNO | AMSGBTN_CANCEL,
	};
	
	enum MessageBoxStyle
	{
		AMSGSTYLE_PLAIN = 0,
		
		AMSGSTYLE_INFORMATION = 1,
		AMSGSTYLE_WARNING = 2,
		AMSGSTYLE_CRITICAL = 3,
		AMSGSTYLE_QUESTION = 4,
		
		AMSGSTYLE_MODAL = 8,
		AMSGSTYLE_TERMINATEAPPONDISPLAY = 16, 
	};
	
	aprilFnExport Window* createAprilWindow(chstr window_system_name, int w, int h, bool fullscreen, chstr title);
	aprilFnExport DEPRECATED_ATTRIBUTE gvec2 getDesktopResolution();
	aprilFnExport SystemInfo& getSystemInfo();
	aprilFnExport MessageBoxButton messageBox(chstr title, chstr text, MessageBoxButton buttonMask = AMSGBTN_OK, MessageBoxStyle style = AMSGSTYLE_PLAIN, hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL);

	aprilExport extern Window* window;

}


#endif
