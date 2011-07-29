/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica                                                       *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef HAVE_MARMELADE


#include <ctype.h> // tolower()

#include "MarmeladeWindow.h"
#include "Keys.h"
#include "Marmelade_RenderSystem.h"

extern "C" int gAprilShouldInvokeQuitCallback;

namespace april
{
	
	MarmeladeWindow::MarmeladeWindow(int w, int h, bool fullscreen, chstr title)
	{
		//april::log("Creating Marmelade Windowsystem");
		
		// postavi ime prozora na title.c_str()
		mTitle = title;
        mCursorInside = true;
		
		// set up display with width w, height h, any bpp, opengl, and optionally fullscreen
		mFullscreen = fullscreen;
		mScreen = (uint16 *)s3eSurfacePtr();
		
		glClear(GL_COLOR_BUFFER_BIT);
		presentFrame();
		if (!mScreen)
		{
			april::log("Requested display mode could not be provided");
			exit(0);
		}
		// we are not running yet
		mRunning = false;
		// cursor is visible by default
		mCursorVisible = true;
	}
	
	//////////////////////
	// implementations
	//////////////////////
	
	void MarmeladeWindow::enterMainLoop()
	{
		mRunning = true;
		// enejblat junikod?
		
		while (mRunning)
		{
			//check if we should quit...
			if (gAprilShouldInvokeQuitCallback)
			{
				uint32 event;
				/*
				event == quit
				dodaj event u event queue
				*/
				gAprilShouldInvokeQuitCallback = 0;
			}
			//first process sdl events
			doEvents();
			_handleDisplayAndUpdate();
		}
				
	}
	
	void MarmeladeWindow::doEvents()
	{
		uint32 event;

		/*while (SDL_PollEvent(&event))
        {
			switch (event.type)
            {
				case SDL_VIDEORESIZE:
					// do a SetVideoMode here, if we really want this
					//g_game->doResize(event.resize.w, event.resize.h);
					break;
				case SDL_QUIT:
					if( handleQuitRequest(true))
					{
						mRunning = false;
					}
					break;
				case SDL_KEYUP:
				case SDL_KEYDOWN:
#ifdef __APPLE__
					// on mac os, we need to handle command+q
					if(SDL_GetModState() & KMOD_META && (tolower(event.key.keysym.unicode) == 'q' || event.key.keysym.sym == SDLK_q))
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
				case SDL_ACTIVEEVENT:
					if (event.active.state & SDL_APPINPUTFOCUS)
					{
						handleFocusEvent(event.active.gain);
					}
					if (event.active.state & SDL_APPMOUSEFOCUS)
					{
						if (event.active.gain && this->isSystemCursorShown() && !mCursorVisible)
						{
							SDL_ShowCursor(0);
						}
						mCursorInside = event.active.gain;
					}
					break;
				default:
					break;
			}
		}*/
		// mac only: extra visibility handling
		this->_platformCursorVisibilityUpdate(mCursorVisible);
	}
	
	MarmeladeWindow::~MarmeladeWindow()
	{
		// quit + cleanup
	}
	
	void MarmeladeWindow::terminateMainLoop()
	{
		mRunning = false;
	}
	
	void MarmeladeWindow::destroyWindow()
	{
		// quit + unisti prozor
	}
	
	void MarmeladeWindow::presentFrame()
	{
		// eglSwapBuffers(g_EGLDisplay, g_EGLSurface);
	}
	
	void MarmeladeWindow::showSystemCursor(bool b)
	{
		mCursorVisible = b;
	}
	
	bool MarmeladeWindow::isSystemCursorShown()
	{
		return mCursorVisible;
	}
	
	void MarmeladeWindow::setWindowTitle(chstr title)
	{
		// postavi naziv prozora na titl.c_str()
		mTitle = title;
	}
	
	gvec2 MarmeladeWindow::getCursorPosition()
	{
		return gvec2(mCursorX,mCursorY);
	}
	
	int MarmeladeWindow::getWidth()
	{
		return s3eSurfaceGetInt(S3E_SURFACE_WIDTH);
	}
	
	int MarmeladeWindow::getHeight()
	{
		return s3eSurfaceGetInt(S3E_SURFACE_HEIGHT);
	}
	
	bool MarmeladeWindow::isCursorInside()
	{
		return mCursorInside;
	}
	
	//////////////////////
	// private parts
	//////////////////////	
	
	void MarmeladeWindow::_handleKeyEvent(Window::KeyEventType type, uint32 keysym, unsigned int unicode)
	{
		april::KeySym akeysym = AK_UNKNOWN;
	
		/*
		#define _s2a(sdlk,ak) case sdlk: akeysym = ak; break; 
		#define s2a(sdlk,ak) _s2a(SDLK_##sdlk, AK_##ak)
		#define sea(key) s2a(key,key)
		*/

		/*
		switch (keysym)
        {
			// control character keys
			s2a(BACKSPACE, BACK);

		case SDLK_DELETE: akeysym = AK_DELETE; //sea(DELETE);

			sea(TAB)
			sea(RETURN);
			s2a(KP_ENTER, RETURN);
			
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
			
			// space
			sea(SPACE);
			
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
			s2a(ESCAPE,ESCAPE);
			
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
			
			s2a(LCTRL,LCONTROL);
		default:
			break;
		}
		// number keys
		if (keysym >= '0' && keysym <= '9')
		{
			akeysym = (KeySym)keysym;
		}
		// letters
		if (keysym >= 'a' && keysym <= 'z') // sdl letter keys are ascii's small letters
		{
			akeysym = (KeySym)(keysym - 32); // april letter keys are ascii's capital letters
		}*/
		//printf("keycode %d unicode %d (%c)\n", keycode, unicode, unicode);
		Window::handleKeyEvent(type, akeysym, unicode);
	}
		
	void MarmeladeWindow::_handleMouseEvent(uint32 event)
	{
		/*mCursorX = event.button.x; 
		mCursorY = event.button.y;
		Window::MouseEventType mouseevt;
		Window::MouseButton mousebtn = AMOUSEBTN_NONE;
		if (event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (event.button.button)
			{
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
		
		switch (event.type)
		{
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
		*/
	}
	
	void MarmeladeWindow::_handleDisplayAndUpdate()
	{
		static unsigned int x = (unsigned int)s3eTimerGetMs();
		float k = ((int)s3eTimerGetMs() - x) / 1000.0f;
		x = (unsigned int)s3eTimerGetMs();
		
		performUpdate(k);
		rendersys->presentFrame();
	}
		
	void* MarmeladeWindow::getIDFromBackend()
	{
		return NULL;
	}

}

#endif
