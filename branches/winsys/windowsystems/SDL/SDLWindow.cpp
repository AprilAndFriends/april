/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#include <SDL/SDL.h>
#include "SDLWindow.h"
#include "Keys.h"
#include "RenderSystem.h"

extern "C" int gAprilShouldInvokeQuitCallback;

namespace April
{
	
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
		SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 ); 
		
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
		
		// key repeat
		SDL_EnableKeyRepeat(100, 50);
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
			
			//check if we should quit...
			if(gAprilShouldInvokeQuitCallback)
			{
				SDL_Event event;
				event.type = SDL_QUIT;
				SDL_PushEvent(&event);	
				gAprilShouldInvokeQuitCallback = 0;
			}
			
			//first process sdl events
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_VIDEORESIZE:
						// do a SetVideoMode here, if we really want this
						//g_game->doResize(event.resize.w, event.resize.h);
						break;
						
					case SDL_QUIT:
						if(handleQuitRequest(true))
						{
							mRunning = false;
						}
						break;
						
					case SDL_KEYUP:
					case SDL_KEYDOWN:
						
#ifdef __APPLE__
						// on mac os, we need to handle command+q
						if(SDL_GetModState() & KMOD_META && event.key.keysym.sym == SDLK_q)
						{
							if(handleQuitRequest(true))
							{
								mRunning = false;
							}
						}
						else
#endif
						{
							_handleKeyEvent(event.type == SDL_KEYUP ? AKEYEVT_UP : AKEYEVT_DOWN,
											event.key.keysym.sym, 
											event.key.keysym.unicode);
						}

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
	
	//////////////////////
	// private parts
	//////////////////////	
	
	void SDLWindow::_handleKeyEvent(Window::KeyEventType type, SDLKey keysym, unsigned int unicode)
	{
		April::KeySyms akeysym = AK_UNKNOWN;
	
		#define _s2a(sdlk,ak) case sdlk: akeysym = ak; break; 
		#define s2a(sdlk,ak) _s2a(SDLK_##sdlk, AK_##ak)
		#define sea(key) s2a(key,key)
		
		switch (keysym) {
				// control character keys
				s2a(BACKSPACE,BACK);
#ifdef __APPLE__
				s2a(DELETE,BACK);
#else
				sea(DELETE);
#endif
				sea(TAB)
				sea(RETURN);
				s2a(KP_ENTER,RETURN);
				
				// control keys above cursor keys
				s2a(PAGEUP, PRIOR);
				s2a(PAGEDOWN, NEXT);
				sea(HOME);
				sea(END);
				sea(INSERT);
				// delete already defined under control chars
				// this is because on mac sdl, delete == backspace
				// for some reason
				
				// cursor keys
				sea(LEFT);
				sea(RIGHT);
				sea(UP);
				sea(DOWN);
				
				// function keys
				sea(F1);
				sea(F2);
				sea(F3);
				sea(F4);
				sea(F5);
				sea(F6);
				sea(F7);
				sea(F8);
				sea(F9);
				sea(F10);
				sea(F11);
				sea(F12);
				
				// keypad keys
				s2a(KP0, NUMPAD0);
				s2a(KP1, NUMPAD1);
				s2a(KP2, NUMPAD2);
				s2a(KP3, NUMPAD3);
				s2a(KP4, NUMPAD4);
				s2a(KP5, NUMPAD5);
				s2a(KP6, NUMPAD6);
				s2a(KP7, NUMPAD7);
				s2a(KP8, NUMPAD8);
				s2a(KP9, NUMPAD9);
			default:
				break;
		}
		
		// number keys
		if(keysym >= '0' && keysym <= '9')
			akeysym = (KeySyms)keysym;
		
		// letters
		if(keysym >= 'a' && keysym <= 'z') // sdl letter keys are ascii's small letters
			akeysym = (KeySyms)(keysym-32); // april letter keys are ascii's capital letters
		
		//printf("keycode %d unicode %d (%c)\n", keycode, unicode, unicode);
		Window::handleKeyEvent(type, akeysym, unicode);
	}
		
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
		
		performUpdate(k);
		presentFrame();
	}
	

}
