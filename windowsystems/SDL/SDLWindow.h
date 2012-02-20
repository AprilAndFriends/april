/// @file
/// @author  Ivan Vucica
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an SDL window.

#ifndef APRIL_SDL_WINDOW_H
#define APRIL_SDL_WINDOW_H

#ifdef HAVE_SDL

#include <SDL/SDL_keysym.h>
#include <hltypes/hstring.h>

#if _SDLGLES
#include <SDL/SDL.h>
#include <SDL/SDL_gles.h>
#endif

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
		void updateOneFrame();
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
		
		DeviceType getDeviceType();

		float mCursorX; // TODO turn into private
		float mCursorY; // TODO turn into private
		
	private:
		void _handleKeyEvent(Window::KeyEventType type, SDLKey keycode, unsigned int unicode);
		void _handleDisplayAndUpdate();
		void _handleMouseEvent(SDL_Event &evt);
		
		SDL_Surface *mScreen;
		bool mRunning;
		bool mCursorVisible;
		bool mCursorInside;
		bool mWindowFocused;
#if _SDLGLES
		SDL_GLES_Context *mGLESContext;
#endif
	};
}

#endif
#endif
