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
		
		while (mRunning) {
			SDL_Event event;
			int beginticks  = SDL_GetTicks();
			
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
						//g_game->keyRelease(event);
						
						break;
						
					case SDL_KEYDOWN:
						//g_game->keyPress(event);
						break;
						
					case SDL_MOUSEBUTTONUP:
					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEMOTION:
						//g_game->mouseEvent(event);
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
		//destroy(); // TODO
		exit(0);
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
		//return glutGet(GLUT_WINDOW_WIDTH);
		return 640;
	}
	
	int SDLWindow::getWindowHeight()
	{
		//return glutGet(GLUT_WINDOW_HEIGHT);
		return 480;
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
		
		Window::handleKeyEvent(type, keycode, unicode);
	}
	
	//////////////////////
	// private parts
	//////////////////////

	void SDLWindow::_handleKeyUp(unsigned char key, int x, int y)
	{
		SDLWindow::_instance->handleKeyEvent(Window::AKEYEVT_UP, key, key>=' ' ? key : 0);
	}
	
	void SDLWindow::_handleKeyDown(unsigned char key, int x, int y)
	{
		/*
		if (key == 27) //esc
		{
			rendersys->terminateMainLoop();
		}
		 */
		
		SDLWindow::_instance->handleKeyEvent(Window::AKEYEVT_DOWN, key, key>=' ' ? key : 0);
	}
	
	void SDLWindow::_handleKeySpecial(int key, int x, int y)
	{
		SDLWindow::_instance->handleKeyEvent(Window::AKEYEVT_DOWN, key, 0);
	}
	
	void SDLWindow::_handleMouseButton(int button, int state, int x,int y)
	{
		SDLWindow::_instance->mCursorX=x; 
		SDLWindow::_instance->mCursorY=y;
		/*if (state == GLUT_DOWN)
			SDLWindow::_instance->handleMouseEvent(Window::AMOUSEEVT_DOWN, x, y, (Window::MouseButton)button);
		else
			SDLWindow::_instance->handleMouseEvent(Window::AMOUSEEVT_UP, x, y, (Window::MouseButton)button);*/
	}
	
	void SDLWindow::_handleMouseMove(int x,int y)
	{
		
		SDLWindow::_instance->mCursorX=x; 
		SDLWindow::_instance->mCursorY=y;
		
		// TODO last argument should be button, as remembered from _handleMouseButton
		// this is because glutMotionFunc() also calls it, and we may want to know which button is active.
		
		SDLWindow::_instance->handleMouseEvent(Window::AMOUSEEVT_MOVE, x, y, (Window::MouseButton)0);
		
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
