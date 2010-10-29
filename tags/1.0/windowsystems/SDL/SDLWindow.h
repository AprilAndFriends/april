/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#ifndef APRIL_SDLWINDOW_H_INCLUDED
#define APRIL_SDLWINDOW_H_INCLUDED

#ifdef HAVE_SDL

#include <hltypes/hstring.h>
#include <SDL/SDL_keysym.h>
#include "Window.h"
#include "AprilExport.h"

struct SDL_Surface;
union SDL_Event;

namespace April
{
	class SDLWindow : public Window
	{
	private:
		
		void _handleKeyEvent(Window::KeyEventType type, SDLKey keycode, unsigned int unicode);
		void _handleDisplayAndUpdate();
		void _handleMouseEvent(SDL_Event &evt);
		SDL_Surface *mScreen;
		bool mRunning;
		bool mCursorVisible;
	public:
		
		SDLWindow(int w, int h, bool fullscreen, chstr title);
		~SDLWindow();
		
		// implementations
		void enterMainLoop();
		void terminateMainLoop();
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWindowWidth();
		int getWindowHeight();
		void setWindowTitle(chstr title);
		gtypes::Vector2 getCursorPos();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		
		
		

		float mCursorX, mCursorY; // TODO turn into private
	};
}

#endif

#endif