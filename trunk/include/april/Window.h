/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica                                                       *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_WINDOW_H
#define APRIL_WINDOW_H

#include <gtypes/Vector2.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	class RenderSystem;
	
	class aprilExport Window 
	{
	public:
		// data types
		enum MouseEventType
		{
			AMOUSEEVT_UP = 0,
			AMOUSEEVT_DOWN,
			AMOUSEEVT_MOVE
		};
		
		enum KeyEventType
		{
			AKEYEVT_UP = 0,
			AKEYEVT_DOWN
		};
		
		enum MouseButton
		{
			AMOUSEBTN_NONE = 0,
			AMOUSEBTN_LEFT,
			AMOUSEBTN_RIGHT,
			AMOUSEBTN_MIDDLE
		};
		enum DeviceOrientation
		{
			ADEVICEORIENTATION_NONE=0,
			ADEVICEORIENTATION_PORTRAIT,
			ADEVICEORIENTATION_PORTRAIT_UPSIDEDOWN,
			ADEVICEORIENTATION_LANDSCAPE_LEFT, // bottom of device is on the left
			ADEVICEORIENTATION_LANDSCAPE_RIGHT, // bottom of device is on the right
			ADEVICEORIENTATION_FACE_DOWN, // screen is facing the ground
			ADEVICEORIENTATION_FACE_UP // screen is facing the sky
		};
		
		// utility funcs
		void _platformCursorVisibilityUpdate(bool visible);
		
		// simple setters
		void setRenderSystem(RenderSystem* rs) { mRenderSystem = rs; }
		
		void setUpdateCallback(bool (*callback)(float));
		void setMouseCallbacks(void (*mouse_dn)(float, float, int),
							   void (*mouse_up)(float, float, int),
							   void (*mouse_move)(float, float));
		void setKeyboardCallbacks(void (*key_dn)(unsigned int),
								  void (*key_up)(unsigned int),
								  void (*char_callback)(unsigned int));
		void setQuitCallback(bool (*quit_callback)(bool can_reject));
		void setWindowFocusCallback(void (*focus_callback)(bool));
		void setVirtualKeyboardCallback(void (*vk_callback)(bool));
		virtual void setDeviceOrientationCallback(void (*vk_callback)(DeviceOrientation)); 
		
		// misc pure virtuals
		virtual void enterMainLoop() = 0;
		virtual void terminateMainLoop() = 0;
		virtual void showSystemCursor(bool b) = 0;
		virtual bool isSystemCursorShown() = 0;
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;
		int getWindowWidth() DEPRECATED_ATTRIBUTE { return getWidth(); }
		int getWindowHeight() DEPRECATED_ATTRIBUTE { return getHeight(); }
		virtual void setWindowTitle(chstr title) = 0;
		virtual gvec2 getCursorPosition() = 0;
		gvec2 getCursorPos() DEPRECATED_ATTRIBUTE { return getCursorPosition(); }
		virtual void presentFrame() = 0;
		virtual void* getIDFromBackend() = 0;
		virtual void doEvents() = 0;
		float getAspectRatio();
		float getWindowAspectRatio() DEPRECATED_ATTRIBUTE { return getAspectRatio(); }
		
		// misc virtuals
		virtual bool isFullscreen();
		virtual void beginKeyboardHandling();
		virtual void terminateKeyboardHandling();
		virtual float prefixRotationAngle();
		
		// generic but overridable event handlers
		virtual void handleMouseEvent(MouseEventType type, float x, float y, MouseButton button);
		virtual void handleKeyEvent(KeyEventType type, KeySym keycode, unsigned int unicode);
		virtual bool handleQuitRequest(bool can_reject);
		virtual void handleFocusEvent(bool has_focus);
		virtual bool performUpdate(float time_increase);
		
	protected:		
		RenderSystem* mRenderSystem;
		hstr mTitle;
		bool mFullscreen;
		
		bool (*mUpdateCallback)(float);
		void (*mMouseDownCallback)(float, float, int);
		void (*mMouseUpCallback)(float, float, int);
		void (*mMouseMoveCallback)(float, float);
		void (*mKeyDownCallback)(unsigned int);
		void (*mKeyUpCallback)(unsigned int);
		void (*mCharCallback)(unsigned int);
		bool (*mQuitCallback)(bool can_reject);
		void (*mFocusCallback)(bool);
		void (*mVKeyboardCallback)(bool);
		void (*mDeviceOrientationCallback)(DeviceOrientation);
		
		Window();
		
	};
	
	enum MessageBoxButton
	{
		AMSGBTN_OK = 1,
		AMSGBTN_CANCEL = 2,
		AMSGBTN_YES = 4,
		AMSGBTN_NO = 8,
		
		AMSGBTN_OKCANCEL = AMSGBTN_OK | AMSGBTN_CANCEL,
		AMSGBTN_YESNO = AMSGBTN_YES | AMSGBTN_NO,
		AMSGBTN_YESNOCANCEL = AMSGBTN_YESNO | AMSGBTN_CANCEL
	};
	
	enum MessageBoxStyle
	{
		AMSGSTYLE_PLAIN = 0,
		
		AMSGSTYLE_INFORMATION = 1,
		AMSGSTYLE_WARNING = 2,
		AMSGSTYLE_CRITICAL = 3,
		AMSGSTYLE_QUESTION = 4,
		
		AMSGSTYLE_MODAL = 8
	};
	
	aprilFnExport Window* createAprilWindow(chstr window_system_name, int w, int h, bool fullscreen, chstr title);
	aprilFnExport gvec2 getDesktopResolution();
	aprilFnExport MessageBoxButton messageBox(chstr title, chstr text, MessageBoxButton buttonMask = AMSGBTN_OK, MessageBoxStyle style = AMSGSTYLE_PLAIN);

}


#endif
