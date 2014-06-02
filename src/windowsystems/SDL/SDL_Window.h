/// @file
/// @version 3.36
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines an SDL window.

#ifdef _SDL_WINDOW
#ifndef APRIL_SDL_WINDOW_H
#define APRIL_SDL_WINDOW_H
#include <SDL/SDL.h>

#ifdef _OPENGLES
#include <SDL/SDL_gles.h>
#endif

#include <hltypes/hstring.h>

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
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();
		
		void setTitle(chstr title);
		bool isCursorVisible();
		HL_DEFINE_IS(cursorInside, CursorInside);
		int getWidth();
		int getHeight();
		gvec2 getCursorPosition();
		void* getBackendId();
		void setResolution(int w, int h, bool fullscreen);

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();
		
	protected:
		bool cursorInside;
		bool scrollHorizontal;
		::SDL_Window* window;
		SDL_Renderer* renderer;
		// TODO - is this still needed in SDL 2?
#ifdef _OPENGLES
		SDL_GLES_Context* glesContext;
#endif

		Cursor* _createCursor();
		void _refreshCursor();

		void _handleSDLKeyEvent(Window::KeyEventType type, SDL_Keycode keyCode, unsigned int unicode);
		void _handleSDLMouseEvent(SDL_Event &evt);
		float _calcTimeSinceLastFrame();

	};

}

#endif
#endif
