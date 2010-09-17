/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_WINDOW_H
#define APRIL_WINDOW_H

#include <hltypes/hstring.h>
#include <gtypes/Vector2.h>
#include "Keys.h"
#include "AprilExport.h"

namespace April
{
	
	class RenderSystem;	
	class AprilExport Window 
	{
	protected:		
		RenderSystem* mRenderSystem;
		hstr mTitle;

		bool (*mUpdateCallback)(float);
		void (*mMouseDownCallback)(float,float,int);
		void (*mMouseUpCallback)(float,float,int);
		void (*mMouseMoveCallback)(float,float);
		void (*mKeyDownCallback)(unsigned int);
		void (*mCharCallback)(unsigned int);
		void (*mKeyUpCallback)(unsigned int);
		bool (*mQuitCallback)(bool can_reject);
		void (*mFocusCallback)(bool);

		Window();
		
	public:
		
		
		// data types
		
		enum MouseEventType
		{
			AMOUSEEVT_UP=0,
			AMOUSEEVT_DOWN,
			AMOUSEEVT_MOVE
		};
		
		enum KeyEventType
		{
			AKEYEVT_UP=0,
			AKEYEVT_DOWN
		};
		
		enum MouseButton
		{
			AMOUSEBTN_NONE=0,
			AMOUSEBTN_LEFT,
			AMOUSEBTN_RIGHT,
			AMOUSEBTN_MIDDLE
		};
		
		
		// simple setters
		void setRenderSystem(RenderSystem* rs) { mRenderSystem = rs; }
		
		void setUpdateCallback(bool (*callback)(float));
		void setMouseCallbacks(void (*mouse_dn)(float,float,int),
							   void (*mouse_up)(float,float,int),
							   void (*mouse_move)(float,float));
		void setKeyboardCallbacks(void (*key_dn)(unsigned int),
								  void (*key_up)(unsigned int),
								  void (*char_callback)(unsigned int));
		void setQuitCallback(bool (*quit_callback)(bool can_reject));
		void setWindowFocusCallback(void (*focus_callback)(bool));
		
		// misc virtuals and pure virtuals
		virtual void enterMainLoop()=0;
		virtual void terminateMainLoop()=0;
		virtual void showSystemCursor(bool b)=0;
		virtual bool isSystemCursorShown()=0;
		virtual int getWindowWidth()=0;
		virtual int getWindowHeight()=0;
		virtual void setWindowTitle(chstr title)=0;
		virtual gtypes::Vector2 getCursorPos()=0;
		virtual void presentFrame()=0;
		virtual void* getIDFromBackend()=0;
		
		// generic but overridable event handlers
		virtual void handleMouseEvent(MouseEventType type, float x, float y, MouseButton button);
		virtual void handleKeyEvent(KeyEventType type, KeySyms keycode, unsigned int unicode);
		virtual bool handleQuitRequest(bool can_reject);
		virtual bool performUpdate(float time_increase);

	};
	
	Window* createAprilWindow(chstr window_system_name, int w, int h, bool fullscreen, chstr title);
	gtypes::Vector2 getDesktopResolution();

}


#endif
