/// @file
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 1.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_SDL

#ifdef __APPLE__
#include <TargetConditionals.h>
#include <OpenGL/gl.h>
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#include <ApplicationServices/ApplicationServices.h>
#endif
#elif _OPENGLES1
#include <GLES/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gl/GL.h>
#endif

#include <SDL/SDL.h>
#include <ctype.h> // tolower()
#include <SDL/SDL_syswm.h>

#include <hltypes/hthread.h>

#include "SDLWindow.h"
#include "Keys.h"
#include "RenderSystem.h"
#include "april.h"

extern bool gAprilShouldInvokeQuitCallback;

namespace april
{
	
	SDLWindow::SDLWindow(int w, int h, bool fullscreen, chstr title)
	{
		log("Creating SDL Window");
#ifndef __APPLE__
		// we want a centered sdl window
		_putenv("SDL_VIDEO_WINDOW_POS");
		_putenv("SDL_VIDEO_CENTERED=1");
#endif
		// initialize SDL subsystems, such as video, audio, timer, ...
		// and immediately set window title
		SDL_Init(SDL_INIT_VIDEO);
		SDL_WM_SetCaption(title.c_str(), title.c_str());
		mTitle = title;
        mCursorInside = true;
		mWindowFocused = true;
		mScrollHorizontal = false;
		
#if !_SDLGLES
		// set up opengl attributes desired for the context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
#else
		SDL_GLES_Init(SDL_GLES_VERSION_1_1);
#endif

		// set up display with width w, height h, any bpp, opengl, and optionally fullscreen
		mFullscreen = fullscreen;
#ifdef _OPENGL
#if !_SDLGLES
		mScreen = SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
#else
		mScreen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE);
		mGLESContext = SDL_GLES_CreateContext();
		SDL_GLES_MakeCurrent(mGLESContext);
#endif
#else
		mScreen = SDL_SetVideoMode(w, h, 0, (fullscreen ? SDL_FULLSCREEN : 0));
#endif
#ifdef _OPENGL
		glClear(GL_COLOR_BUFFER_BIT);
#endif
		presentFrame();
		if (!mScreen)
		{
#ifdef __APPLE__
#if !TARGET_OS_IPHONE
			// TODO elsewhere, add support for platform-specific msgbox code
			//NSRunAlertPanel(@"Could not open display", @"Game could not set the screen resolution. Perhaps resetting game configuration will help.", @"Ok", nil, nil);
#endif
#endif
			april::log("Requested display mode could not be provided");
			exit(0);
		}
		// we are not running yet
		mRunning = false;
		// cursor is visible by default
		mCursorVisible = true;
		// key repeat
		SDL_EnableKeyRepeat(100, 50);
		doEvents();
		SDL_EnableUNICODE(1);
	}
	
	//////////////////////
	// implementations
	//////////////////////
	
	void SDLWindow::enterMainLoop()
	{
		mRunning = true;
		
		while (mRunning)
		{
			if (!updateOneFrame())
			{
				mRunning = false;
			}
		}
				
	}
	
	bool SDLWindow::updateOneFrame()
	{
		//check if we should quit...
		if (gAprilShouldInvokeQuitCallback)
		{
			SDL_Event event;
			event.type = SDL_QUIT;
			SDL_PushEvent(&event);	
			gAprilShouldInvokeQuitCallback = false;
		}
		//first process sdl events
		doEvents();
		return _handleDisplayAndUpdate();
	}

	
	void SDLWindow::doEvents()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
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
						mWindowFocused = event.active.gain;
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
		}
		// mac only: extra visibility handling
		this->_platformCursorVisibilityUpdate(mCursorVisible);
	}
	
	SDLWindow::~SDLWindow()
	{
#if _SDLGLES
		SDL_GLES_DeleteContext(mGLESContext);
		SDL_GLES_Quit();
#endif
		SDL_Quit();
	}
	
	void SDLWindow::terminateMainLoop()
	{
		mRunning = false;
	}
	
	void SDLWindow::destroyWindow()
	{
		SDL_Quit();
	}
	
	void SDLWindow::presentFrame()
	{
#ifdef _OPENGL
#if !_SDLGLES
		SDL_GL_SwapBuffers();
#else
		SDL_GLES_SwapBuffers();
#endif
#endif
	}
	
	void SDLWindow::showSystemCursor(bool b)
	{
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		// intentionally do nothing; let the platform specific code take over
		// if sdl-hidden mouse goes over the dock, then it will show again,
		// and won't be hidden again
#else
		// on other platforms, SDL does a good enough job
		SDL_ShowCursor(b ? 1 : 0);
#endif
		mCursorVisible = b;
	}
	
	bool SDLWindow::isSystemCursorShown()
	{
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		return CGCursorIsVisible();
#else
		return SDL_ShowCursor(SDL_QUERY);
#endif
	}
	
	void SDLWindow::setWindowTitle(chstr title)
	{
		SDL_WM_SetCaption(title.c_str(), title.c_str());
		mTitle = title;
	}
	
	gvec2 SDLWindow::getCursorPosition()
	{
		return gvec2(mCursorX, mCursorY);
	}
	
	int SDLWindow::getWidth()
	{
		return SDL_GetVideoInfo()->current_w;
	}
	
	int SDLWindow::getHeight()
	{
		return SDL_GetVideoInfo()->current_h;
	}
	
	bool SDLWindow::isCursorInside()
	{
		return mCursorInside;
	}
	
	//////////////////////
	// private parts
	//////////////////////	
	
	void SDLWindow::_handleKeyEvent(Window::KeyEventType type, SDLKey keysym, unsigned int unicode)
	{
		april::KeySym akeysym = AK_UNKNOWN;
	
		#define _s2a(sdlk,ak) case sdlk: akeysym = ak; break; 
		#define s2a(sdlk,ak) _s2a(SDLK_##sdlk, AK_##ak)
		#define sea(key) s2a(key,key)
		
		switch (keysym)
        {
			// control character keys
			s2a(BACKSPACE, BACK);
#ifdef __APPLE__
			s2a(DELETE, BACK);
#else
		case SDLK_DELETE: akeysym = AK_DELETE; //sea(DELETE);
#endif
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
			s2a(RCTRL,RCONTROL);
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
		}
		if (akeysym == AK_LCONTROL || akeysym == AK_RCONTROL)
		{
			if (type == AKEYEVT_DOWN)
			{
				mScrollHorizontal = true;
			}
			else if (type == AKEYEVT_UP)
			{
				mScrollHorizontal = false;
			}
		}
		//printf("keycode %d unicode %d (%c)\n", keycode, unicode, unicode);
		Window::handleKeyEvent(type, akeysym, unicode);
	}
		
	void SDLWindow::_handleMouseEvent(SDL_Event &event)
	{
		mCursorX = event.button.x; 
		mCursorY = event.button.y;
		Window::MouseEventType mouseevt;
		Window::MouseButton mousebtn = AMOUSEBTN_NONE;

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
			case SDL_BUTTON_WHEELUP:
				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					mousebtn = AMOUSEBTN_WHEELUP;
					mouseevt = AMOUSEEVT_SCROLL;
				}
				break;
			case SDL_BUTTON_WHEELDOWN:
				if (event.type == SDL_MOUSEBUTTONDOWN)
				{
					mousebtn = AMOUSEBTN_WHEELDN;
					mouseevt = AMOUSEEVT_SCROLL;
				}
				break;
			default:
				mousebtn = AMOUSEBTN_NONE;
				break;
			}
		}
		
		if (mousebtn == AMOUSEBTN_WHEELUP || mousebtn == AMOUSEBTN_WHEELDN)
		{
			float x = (!mScrollHorizontal ? 0.0f : (mousebtn == AMOUSEBTN_WHEELUP ? -1.0f : 1.0f));
			float y = (mScrollHorizontal ? 0.0f : (mousebtn == AMOUSEBTN_WHEELUP ? -1.0f : 1.0f));
			handleMouseEvent(mouseevt, x, y, mousebtn);
		}
		else
		{
			handleMouseEvent(mouseevt, event.button.x, event.button.y, mousebtn);
		}
	}
	
	bool SDLWindow::_handleDisplayAndUpdate()
	{
		static unsigned int x = SDL_GetTicks();
		float k = (SDL_GetTicks() - x) / 1000.0f;
		x = SDL_GetTicks();
		
		if (!mWindowFocused)
			hthread::sleep(100);
		bool result = performUpdate(k);
		rendersys->presentFrame();
		return result;
	}
		
	void* SDLWindow::getIDFromBackend()
	{
#ifdef _WIN32
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWMInfo(&wmInfo);
		return (void*)wmInfo.window;
#else
		// unimplemented
		return NULL;
#endif
	}
	
	Window::DeviceType SDLWindow::getDeviceType()
	{
#if TARGET_OS_MAC
		return DEVICE_MAC_PC;
#elif defined(_WIN32)
		return DEVICE_WINDOWS_PC;
#else
		return DEVICE_LINUX_PC;
#endif
	}
	
#ifndef _WIN32 // hack to prevent multiple definitions of this function under Win32
	SystemInfo& getSystemInfo()
	{
		static SystemInfo info;
		if (info.locale == "")
		{
			info.ram = 1024;
			info.locale = "en";
			info.max_texture_size = 0;
		}
#ifdef _OPENGL
		if (info.max_texture_size == 0 && april::rendersys != NULL)
		{
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &info.max_texture_size);
		}
#endif
		return info;
	}
#endif

}

#endif
