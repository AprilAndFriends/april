/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#include <SDL/SDL.h>
#include "SDLWindow.h"
#include "Keys.h"
#include "RenderSystem.h"

namespace April
{

	
	
	SDLWindow* SDLWindow::_instance;
	
	SDLWindow::SDLWindow(int w, int h, bool fullscreen, chstr title)
	{
		// we want a centered sdl window
		putenv("SDL_VIDEO_WINDOW_POS");
		putenv("SDL_VIDEO_CENTERED=1");
		
		// initialize SDL subsystems, such as video, audio, timer, ...
		// and immediately set window title
		SDL_Init(SDL_INIT_VIDEO);
		SDL_WM_SetCaption(title.c_str(), title.c_str());
		
		// set up opengl attributes desired for the context
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		
		// set up display with width w, height h, any bpp, opengl, and optionally fullscreen
		mScreen = SDL_SetVideoMode(w, h, 0, SDL_OPENGL|(fullscreen ? SDL_FULLSCREEN : 0));
		
		/*
		 #ifdef _WIN32
		 hWnd = FindWindow("GLUT", title.c_str());
		 SetFocus(hWnd);
		 #endif
		 */

		// we are not running yet
		mRunning = false;
		
		_instance = this;
	}
	
	//////////////////////
	// implementations
	//////////////////////
	
	void SDLWindow::enterMainLoop()
	{
		mRunning = true;
		
		SDL_EnableUNICODE(1);
		
		while (mRunning) {
			SDL_Event event;
			
			//first process sdl events
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_VIDEORESIZE:
						// do a SetVideoMode here, if we really want this
						//g_game->doResize(event.resize.w, event.resize.h);
						break;
						
					case SDL_QUIT:
						mRunning = false;
						break;
						
					case SDL_KEYUP:
					case SDL_KEYDOWN:
						handleKeyEvent(event.type == SDL_KEYUP ? AKEYEVT_UP : AKEYEVT_DOWN,
									   event.key.keysym.scancode, 
									   event.key.keysym.unicode);
						break;
						
					case SDL_MOUSEBUTTONUP:
					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEMOTION:
						_handleMouseEvent(event);
						break;
						
						
					default:
						break;
				}
			}
			
			
			_handleDisplayAndUpdate();
			
			
		}
				
	}
	
	void SDLWindow::terminateMainLoop()
	{
		mRunning = false;
	}
	
	
	void SDLWindow::presentFrame()
	{
		SDL_GL_SwapBuffers();
	}
	
	
	void SDLWindow::showSystemCursor(bool b)
	{
		SDL_ShowCursor(b ? 1 : 0);
	}
	
	bool SDLWindow::isSystemCursorShown()
	{
		//SDL_Sh
		//int cursor=glutGet(GLUT_WINDOW_CURSOR);
		//return (cursor == GLUT_CURSOR_NONE) ? 0 : 1;
		return true;
	}
	
	
	void SDLWindow::setWindowTitle(chstr title)
	{
		SDL_WM_SetCaption(title.c_str(), title.c_str());
	}
	
	gtypes::Vector2 SDLWindow::getCursorPos()
	{
		return gtypes::Vector2(mCursorX,mCursorY);
	}
	
	int SDLWindow::getWindowWidth()
	{
		return SDL_GetVideoInfo()->current_w;
	}
	
	int SDLWindow::getWindowHeight()
	{
		return SDL_GetVideoInfo()->current_h;
	}
	
	
	
	
	
	/////////////////////////
	// overrides
	/////////////////////////
	
	void SDLWindow::handleKeyEvent(Window::KeyEventType type, unsigned int keycode, unsigned int unicode)
	{
		/*if (keycode == 9) 
			keycode=AK_TAB;
		else 
			if (keycode >= GLUT_KEY_F1 && keycode <= GLUT_KEY_F12)  // function keys
				keycode+=0x6F;
		*/
		printf("keycode %d unicode %d (%c)\n", keycode, unicode, unicode);
		Window::handleKeyEvent(type, keycode, unicode);
	}
	
	//////////////////////
	// private parts
	//////////////////////
	
	void SDLWindow::_handleMouseEvent(SDL_Event &event)
	{
		mCursorX=event.button.x; 
		mCursorY=event.button.y;
		
		Window::MouseEventType mouseevt;
		Window::MouseButton mousebtn = AMOUSEBTN_NONE;
		
		if(event.type == SDL_MOUSEBUTTONUP ||
		   event.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (event.button.button) {
				case SDL_BUTTON_LEFT:
					mousebtn = AMOUSEBTN_LEFT;
					break;
				case SDL_BUTTON_RIGHT:
					mousebtn = AMOUSEBTN_RIGHT;
					break;
				case SDL_BUTTON_MIDDLE:
					mousebtn = AMOUSEBTN_MIDDLE;
					break;
				default:
					mousebtn = AMOUSEBTN_NONE;
					break;
			}
		}
		
		switch (event.type) {
			case SDL_MOUSEBUTTONUP:
				mouseevt = AMOUSEEVT_UP;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouseevt = AMOUSEEVT_DOWN;
				break;
			case SDL_MOUSEMOTION:
				mouseevt = AMOUSEEVT_MOVE;
				break;
			default:
				break;
		}
		
		
		handleMouseEvent(mouseevt, 
						 event.button.x, event.button.y,
						 mousebtn);
		
		
	}
	
	
	void SDLWindow::_handleDisplayAndUpdate()
	{
		static unsigned int x=SDL_GetTicks();
		float k=(SDL_GetTicks()-x)/1000.0f;
		x=SDL_GetTicks();
		
		SDLWindow::_instance->performUpdate(k);
		SDLWindow::_instance->presentFrame();
	}
	

}
