/// @file
/// @author  Ivan Vucica
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_SDL

#ifdef __APPLE__
#include <TargetConditionals.h>
#include <OpenGL/gl.h>
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

#include "april.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SDL_Window.h"

extern int gAprilShouldInvokeQuitCallback;

namespace april
{
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
	void platform_cursorVisibilityUpdate();
	bool platform_CursorIsVisible();
#endif
	
	SDL_Window::SDL_Window() : Window()
	{
		this->name = APRIL_WS_SDL;
		// centered SDL window
#ifndef __APPLE__
		SDL_putenv("SDL_VIDEO_WINDOW_POS");
		SDL_putenv("SDL_VIDEO_CENTERED=1");
#endif
		this->cursorInside = true;
		this->scrollHorizontal = false;
		this->screen = NULL;
#ifdef _OPENGLES1
		this->glesContext = NULL;
#endif
	}
	
	SDL_Window::~SDL_Window()
	{
		this->destroy();
	}
	
	bool SDL_Window::create(int w, int h, bool fullscreen, chstr title)
	{
		if (!Window::create(w, h, fullscreen, title))
		{
			return false;
		}
		this->cursorInside = true;
		// initialize only SDL video subsystem
		SDL_Init(SDL_INIT_VIDEO);
		// and immediately set window title
		SDL_WM_SetCaption(this->title.c_str(), this->title.c_str());

#ifdef _OPENGL
#ifndef _OPENGLES1
		// set up opengl attributes desired for the context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
		this->screen = SDL_SetVideoMode(w, h, 0, SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
#else
		SDL_GLES_Init(SDL_GLES_VERSION_1_1);
		this->screen = SDL_SetVideoMode(0, 0, 16, SDL_SWSURFACE);
		this->glesContext = SDL_GLES_CreateContext();
		SDL_GLES_MakeCurrent(this->glesContext);
#endif
		glClear(GL_COLOR_BUFFER_BIT);
#else
		this->screen = SDL_SetVideoMode(w, h, 0, (fullscreen ? SDL_FULLSCREEN : 0));
#endif

		if (this->screen == NULL)
		{
#ifdef __APPLE__
#if !TARGET_OS_IPHONE
			// TODO elsewhere, add support for platform-specific msgbox code
			//NSRunAlertPanel(@"Could not open display", @"Game could not set the screen resolution. Perhaps resetting game configuration will help.", @"Ok", nil, nil);
#endif
#endif
			april::log("SDL: requested display mode could not be provided");
			exit(0);
		}
#ifndef _WIN32
		april::rendersys->clear();
		april::rendersys->presentFrame();
#endif
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		// not running yet
		this->running = false;
		// cursor is visible by default
		this->cursorVisible = true;
		// key repeat
		SDL_EnableKeyRepeat(100, 50);
		this->checkEvents();
		SDL_EnableUNICODE(1);
		return true;
	}
	
	bool SDL_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
#ifdef _OPENGLES1
		SDL_GLES_DeleteContext(this->glesContext);
		SDL_GLES_Quit();
#endif
		SDL_Quit();
		return true;
	}
	
	void SDL_Window::setTitle(chstr value)
	{
		Window::setTitle(value);
		SDL_WM_SetCaption(this->title.c_str(), this->title.c_str());
	}
	
	bool SDL_Window::isCursorVisible()
	{
		// TODO - refactor
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		return platform_CursorIsVisible();
#else
		return (SDL_ShowCursor(SDL_QUERY) != 0);
#endif
	}
	
	void SDL_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		// TODO - refactor
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		// intentionally do nothing; let the platform specific code take over
		// if sdl-hidden mouse goes over the dock, then it will show again,
		// and won't be hidden again
#else
		// on other platforms, SDL does a good enough job
		SDL_ShowCursor(this->cursorVisible ? 1 : 0);
#endif
	}
	
	int SDL_Window::getWidth()
	{
		return SDL_GetVideoInfo()->current_w;
	}
	
	int SDL_Window::getHeight()
	{
		return SDL_GetVideoInfo()->current_h;
	}
	
	void* SDL_Window::getBackendId()
	{
#ifdef _WIN32
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWMInfo(&wmInfo);
		return (void*)wmInfo.window;
#else
		// not implemented
		return NULL;
#endif
	}
	
	bool SDL_Window::updateOneFrame()
	{
		// check if we should quit...
		if (gAprilShouldInvokeQuitCallback != 0)
		{
			SDL_Event sdlEvent;
			sdlEvent.type = SDL_QUIT;
			SDL_PushEvent(&sdlEvent);
			gAprilShouldInvokeQuitCallback = 0;
		}
		// first process sdl events
		this->checkEvents();
		return this->_handleDisplayAndUpdate();
	}

	void SDL_Window::terminateMainLoop()
	{
		this->running = false;
	}
	
	void SDL_Window::checkEvents()
	{
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
			case SDL_VIDEORESIZE:
				// do a SetVideoMode here, if we really want this
				//g_game->doResize(event.resize.w, event.resize.h);
				break;
			case SDL_QUIT:
				if (this->handleQuitRequest(true))
				{
					this->running = false;
				}
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
#ifdef __APPLE__
				// on mac os, we need to handle command+q
				if (SDL_GetModState() & KMOD_META && (tolower(sdlEvent.key.keysym.unicode) == 'q' || sdlEvent.key.keysym.sym == SDLK_q))
				{
					if (this->handleQuitRequest(true))
					{
						this->running = false;
					}
				}
				else
#endif
				{
					this->_handleKeyEvent((sdlEvent.type == SDL_KEYUP ? AKEYEVT_UP : AKEYEVT_DOWN),
						sdlEvent.key.keysym.sym, sdlEvent.key.keysym.unicode);
				}
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
				this->_handleMouseEvent(sdlEvent);
				break;
			case SDL_ACTIVEEVENT:
				if (sdlEvent.active.state & SDL_APPINPUTFOCUS)
				{
					this->handleFocusChangeEvent(sdlEvent.active.gain != 0);
				}
				if (sdlEvent.active.state & SDL_APPMOUSEFOCUS)
				{
					if (sdlEvent.active.gain && this->isCursorVisible() && !this->cursorVisible)
					{
						SDL_ShowCursor(0);
					}
					this->cursorInside = (sdlEvent.active.gain != 0);
				}
				break;
			default:
				break;
			}
		}
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		platform_cursorVisibilityUpdate();
#endif
	}
	
	void SDL_Window::presentFrame()
	{
#ifdef _OPENGL
#ifndef _OPENGLES1
		SDL_GL_SwapBuffers();
#else
		SDL_GLES_SwapBuffers();
#endif
#endif
	}
	
	void SDL_Window::_handleKeyEvent(Window::KeyEventType type, SDLKey keysym, unsigned int unicode)
	{
		april::KeySym akeysym = AK_UNKNOWN;
	
		#define _s2a(sdlk, ak) case sdlk: akeysym = ak; break; 
		#define s2a(sdlk, ak) _s2a(SDLK_ ## sdlk, AK_ ## ak)
		#define sea(key) s2a(key, key)
		
		switch (keysym)
		{
			// control character keys
			s2a(BACKSPACE, BACK);
#ifdef __APPLE__
			s2a(DELETE, BACK);
#else
		case SDLK_DELETE:
			akeysym = AK_DELETE; //sea(DELETE);
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
			s2a(ESCAPE, ESCAPE);
			
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
			
			s2a(LCTRL, LCONTROL);
			s2a(RCTRL, RCONTROL);
		default:
			break;
		}
		// number keys
		if (keysym >= '0' && keysym <= '9')
		{
			akeysym = (KeySym)keysym;
		}
		// letters
		if (keysym >= 'a' && keysym <= 'z') // sdl letter keys are small ASCII letters
		{
			akeysym = (KeySym)(keysym - 32); // april letter keys are capital ASCII letters
		}
		if (akeysym == AK_LCONTROL || akeysym == AK_RCONTROL)
		{
			if (type == AKEYEVT_DOWN)
			{
				this->scrollHorizontal = true;
			}
			else if (type == AKEYEVT_UP)
			{
				this->scrollHorizontal = false;
			}
		}
		Window::handleKeyEvent(type, akeysym, unicode);
	}
		
	void SDL_Window::_handleMouseEvent(SDL_Event& sdlEvent)
	{
		this->cursorPosition.set(sdlEvent.button.x, sdlEvent.button.y);
		Window::MouseEventType mouseEvent;
		Window::MouseButton mouseButton = AMOUSEBTN_NONE;

		switch (sdlEvent.type)
		{
		case SDL_MOUSEBUTTONUP:
			mouseEvent = AMOUSEEVT_UP;
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouseEvent = AMOUSEEVT_DOWN;
			break;
		case SDL_MOUSEMOTION:
			mouseEvent = AMOUSEEVT_MOVE;
			break;
		default:
			break;
		}

		if (sdlEvent.type == SDL_MOUSEBUTTONUP || sdlEvent.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (sdlEvent.button.button)
			{
			case SDL_BUTTON_LEFT:
				mouseButton = AMOUSEBTN_LEFT;
				break;
			case SDL_BUTTON_RIGHT:
				mouseButton = AMOUSEBTN_RIGHT;
				break;
			case SDL_BUTTON_MIDDLE:
				mouseButton = AMOUSEBTN_MIDDLE;
				break;
			case SDL_BUTTON_WHEELUP:
				if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
				{
					mouseButton = AMOUSEBTN_WHEELUP;
					mouseEvent = AMOUSEEVT_SCROLL;
				}
				break;
			case SDL_BUTTON_WHEELDOWN:
				if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
				{
					mouseButton = AMOUSEBTN_WHEELDOWN;
					mouseEvent = AMOUSEEVT_SCROLL;
				}
			default:
				mouseButton = AMOUSEBTN_NONE;
				break;
			}
		}
		
		if (mouseButton == AMOUSEBTN_WHEELUP || mouseButton == AMOUSEBTN_WHEELDOWN)
		{
			gvec2 scroll;
			scroll.x = (!this->scrollHorizontal ? 0.0f : (mouseButton == AMOUSEBTN_WHEELUP ? -1.0f : 1.0f));
			scroll.y = (this->scrollHorizontal ? 0.0f : (mouseButton == AMOUSEBTN_WHEELUP ? -1.0f : 1.0f));
			this->handleMouseEvent(mouseEvent, scroll, mouseButton);
		}
		else
		{
			this->handleMouseEvent(mouseEvent, this->cursorPosition, mouseButton);
		}
	}
	
	bool SDL_Window::_handleDisplayAndUpdate()
	{
		static unsigned int x = SDL_GetTicks();
		float k = (SDL_GetTicks() - x) / 1000.0f;
		x = SDL_GetTicks();
		if (!this->focused)
		{
			hthread::sleep(100);
		}
		bool result = this->performUpdate(k);
		april::rendersys->presentFrame();
		return result;
	}
		
}

#endif
