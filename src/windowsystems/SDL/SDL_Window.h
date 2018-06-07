/// @file
/// @version 5.1
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
		
		void setTitle(chstr title);
		bool isCursorVisible() const;
		HL_DEFINE_IS(cursorInside, CursorInside);
		int getWidth(const);
		int getHeight() const;
		gvec2f getCursorPosition() const;
		void* getBackendId() const;

		bool update(float timeDelta);
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

		void _systemCreate(int w, int h, bool fullscreen, chstr title, Window::Options options);
		void _systemDestroy();
		
		Cursor* _createCursor(bool fromResource);
		void _refreshCursor();

		void _systemSetResolution(int width, int height, bool fullscreen);
		
		void _presentFrame(bool systemEnabled);
		
		void _handleSDLKeyEvent(Window::KeyEventType type, SDL_Keycode keyCode, unsigned int unicode);
		void _handleSDLMouseEvent(SDL_Event &evt);

	};

}

#endif
#endif
