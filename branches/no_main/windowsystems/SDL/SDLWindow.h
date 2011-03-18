/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica                                                       *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_SDL_WINDOW_H
#define APRIL_SDL_WINDOW_H

#ifdef HAVE_SDL

#include <SDL/SDL_keysym.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Window.h"

struct SDL_Surface;
union SDL_Event;

namespace april
{
	class SDLWindow : public Window
	{
	public:
		SDLWindow(int w, int h, bool fullscreen, chstr title);
		~SDLWindow();
		
		// implementations
		void enterMainLoop();
		void terminateMainLoop();
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWidth();
		int getHeight();
		void setWindowTitle(chstr title);
		gvec2 getCursorPosition();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();

		float mCursorX; // TODO turn into private
		float mCursorY; // TODO turn into private
		
	private:
		void _handleKeyEvent(Window::KeyEventType type, SDLKey keycode, unsigned int unicode);
		void _handleDisplayAndUpdate();
		void _handleMouseEvent(SDL_Event &evt);
		SDL_Surface *mScreen;
		bool mRunning;
		bool mCursorVisible;
		
	};
}

#endif
#endif
