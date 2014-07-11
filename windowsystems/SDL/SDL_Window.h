/// @file
/// @author  Ivan Vucica
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
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

#ifdef _OPENGLES
#include <SDL/SDL.h>
#include <SDL/SDL_gles.h>
#endif

#include "aprilExport.h"
#include "Window.h"

struct SDL_Surface;
union SDL_Event;

namespace april
{
	class SDL_Window : public Window
	{
	public:
		SDL_Window();
		~SDL_Window();
		bool create(int w, int h, bool fullscreen, chstr title, chstr options = "");
		bool destroy();
		
		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool visible);
		HL_DEFINE_IS(bool, cursorInside, CursorInside);
		int getWidth();
		int getHeight();
		bool isTouchEnabled() { return false; }
		void setTouchEnabled(bool value) { }
		gvec2 getCursorPosition();
		void* getBackendId();

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();
		
	protected:
		bool cursorInside;
		bool scrollHorizontal;
		SDL_Surface* screen;
#ifdef _OPENGLES
		SDL_GLES_Context* glesContext;
#endif

		float _calcTimeSinceLastFrame();
		void _handleKeyEvent(Window::KeyEventType type, SDLKey keyCode, unsigned int unicode);
		void _handleMouseEvent(SDL_Event &evt);		

	};

}

#endif
#endif
