/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#ifndef APRIL_WIN32WINDOW_H_INCLUDED
#define APRIL_WIN32WINDOW_H_INCLUDED

#include <hltypes/hstring.h>
#include "Window.h"
#include "AprilExport.h"

namespace April
{
	class AprilExport Win32Window : public Window
	{
	private:
		
		//void _handleKeyEvent(Window::KeyEventType type, SDLKey keycode, unsigned int unicode);
		//void _handleDisplayAndUpdate();
		//void _handleMouseEvent(SDL_Event &evt);
		//SDL_Surface *mScreen;
		bool mRunning;
		//bool mCursorVisible;
		bool mActive;
	public:
		
		Win32Window(int w, int h, bool fullscreen, chstr title);
		~Win32Window();
		
		void _setActive(bool active) { mActive=active; }
		
		// implementations
		void enterMainLoop();
		void terminateMainLoop();
		bool isRunning() { return mRunning; }
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWindowWidth();
		int getWindowHeight();
		void setWindowTitle(chstr title);
		gtypes::Vector2 getCursorPos();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		
		// event handlers
		void triggerKeyEvent(bool down,unsigned int keycode);
		void triggerCharEvent(unsigned int chr);
	
		void triggerMouseUpEvent(int button);
		void triggerMouseDownEvent(int button);
		void triggerMouseMoveEvent();
		bool triggerQuitEvent();
		void triggerFocusCallback(bool focused);
		
		

		float mCursorX, mCursorY; // TODO turn into private
	};
}

#endif
