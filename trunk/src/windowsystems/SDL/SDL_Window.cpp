/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _SDL_WINDOW
#include <hltypes/hplatform.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#include <OpenGL/gl.h>
#elif _OPENGLES
#include <GLES/gl.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gl/GL.h>
#endif

#include <SDL/SDL.h>
#include <ctype.h> // tolower()
#include <SDL/SDL_syswm.h>

#include <hltypes/hexception.h>
#include <hltypes/hlog.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SDL_Cursor.h"
#include "SDL_Window.h"
#ifdef _DIRECTX
#include "DirectX_RenderSystem.h"
#endif
#ifdef _OPENGL1
#include "OpenGL1_RenderSystem.h"
#endif
#ifdef _OPENGLES
#include "OpenGLES_RenderSystem.h"
#endif

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
		// TODO - is this still needed in SDL 2?
		/*
#ifndef __APPLE__
		SDL_setenv("SDL_VIDEO_WINDOW_POS", "1", 1);
		SDL_setenv("SDL_VIDEO_CENTERED", "1", 1);
#endif
		*/
		this->cursorInside = true;
		this->scrollHorizontal = false;
		this->window = NULL;
		this->renderer = NULL;
		this->created = false;
		// TODO - is this still needed in SDL 2?
#ifdef _OPENGLES
		this->glesContext = NULL;
#endif
		this->cursorExtensions += ".png";
	}
	
	SDL_Window::~SDL_Window()
	{
		this->destroy();
	}
	
	bool SDL_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->inputMode = MOUSE;
		this->cursorInside = true;
		// initialize only SDL video subsystem
		SDL_Init(SDL_INIT_VIDEO);
		int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
		if (fullscreen)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}
		if (this->options.resizable)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}
#ifdef _OPENGL
		harray<hstr> renderSystems;
		renderSystems += APRIL_RS_OPENGL1;
		renderSystems += APRIL_RS_OPENGLES1;
		renderSystems += APRIL_RS_OPENGLES2;
		if (renderSystems.contains(april::rendersys->getName()))
		{
			flags |= SDL_WINDOW_OPENGL;
		}
#endif
		this->window = SDL_CreateWindow(this->title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
		if (this->window == NULL)
		{
			hlog::error(april::logTag, "SDL window could not be created!");
			return false;
		}
		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (this->renderer == NULL)
		{
			SDL_DestroyWindow(this->window);
			this->window = NULL;
			hlog::error(april::logTag, "SDL renderer could not be created!");
			return false;
		}
		// not running yet
		this->running = false;
		// cursor is visible by default
		this->cursorVisible = true;
		this->created = true;
		return true;
	}
	
	bool SDL_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		SDL_DestroyRenderer(this->renderer);
		this->renderer = NULL;
		SDL_DestroyWindow(this->window);
		this->window = NULL;
		SDL_Quit();
		return true;
	}
	
	void SDL_Window::setTitle(chstr value)
	{
		Window::setTitle(value);
		SDL_SetWindowTitle(this->window, this->title.c_str());
	}
	
	bool SDL_Window::isCursorVisible()
	{
		// TODO - is this still needed in SDL 2?
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		return this->cursorVisible;
#else
		return (SDL_ShowCursor(SDL_QUERY) != 0);
#endif
	}
	
	void SDL_Window::_refreshCursor()
	{
		SDL_ShowCursor(this->cursorVisible ? 1 : 0);
		::SDL_Cursor* cursor = NULL;
		if (this->isCursorVisible() && this->cursor != NULL)
		{
			cursor = ((SDL_Cursor*)this->cursor)->getCursor();
		}
		SDL_SetCursor(cursor);
	}

	int SDL_Window::getWidth()
	{
		int w = 0;
		int h = 0;
		SDL_GetWindowSize(this->window, &w, &h);
		return w;
	}
	
	int SDL_Window::getHeight()
	{
		int w = 0;
		int h = 0;
		SDL_GetWindowSize(this->window, &w, &h);
		return h;
	}
	
	void* SDL_Window::getBackendId()
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(this->window, &wmInfo);
		if (wmInfo.subsystem == SDL_SYSWM_WINDOWS)
		{
			return (void*)wmInfo.info.win.window;
		}
		return NULL;
	}
	
	void SDL_Window::setResolution(int w, int h, bool fullscreen)
	{
		if (this->fullscreen == fullscreen && this->getWidth() == w && this->getHeight() == h)
		{
			return;
		}
		SDL_SetWindowFullscreen(this->window, fullscreen ? 1 : 0);
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
		return Window::updateOneFrame();
	}

	void SDL_Window::checkEvents()
	{
		SDL_Event sdlEvent;
		std::basic_string<unsigned int> text;
		while (SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
			case SDL_WINDOWEVENT:
				switch (sdlEvent.window.event)
				{
				case SDL_WINDOWEVENT_RESIZED:
					SDL_SetWindowSize(this->window, sdlEvent.window.data1, sdlEvent.window.data2);
					this->_setRenderSystemResolution(sdlEvent.window.data1, sdlEvent.window.data2, this->fullscreen);
					break;
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					this->handleFocusChangeEvent(true);
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					this->handleFocusChangeEvent(false);
					break;
				case SDL_WINDOWEVENT_ENTER:
					if (this->isCursorVisible() && !this->cursorVisible)
					{
						SDL_ShowCursor(0);
					}
					this->cursorInside = true;
					break;
				case SDL_WINDOWEVENT_LEAVE:
					this->cursorInside = false;
					break;
				default:
					break;
				}
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
				// TODO - needs to be changed for SDL 2
				// on mac os, we need to handle command+q
				if (SDL_GetModState() & KMOD_META && (tolower(sdlEvent.key.keysym.unicode) == 'q' || sdlEvent.key.keysym.sym == SDLK_q))
				{
					if (this->handleQuitRequest(true))
					{
						this->running = false;
					}
				}
				else
#elif defined(_WIN32)
				if (SDL_GetModState() & KMOD_ALT && sdlEvent.key.keysym.sym == SDLK_KP_ENTER)
				{
					this->toggleHotkeyFullscreen();
				}
				else
#endif
				{
					this->_handleSDLKeyEvent((sdlEvent.type == SDL_KEYUP ? KEY_UP : KEY_DOWN), sdlEvent.key.keysym.sym, 0);
				}
				break;
			case SDL_TEXTINPUT:
				text = hstr(sdlEvent.text.text).u_str();
				for_itert (unsigned int, i, 0, text.size())
				{
					this->_handleSDLKeyEvent(KEY_DOWN, 0, text[i]);
				}
				break;
			case SDL_TEXTEDITING:
				// TODO - needs to be implemented
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
				this->_handleSDLMouseEvent(sdlEvent);
				break;
			default:
				break;
			}
		}
		// TODO - is this still needed in SDL 2?
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		platform_cursorVisibilityUpdate();
#endif
	}
	
	void SDL_Window::presentFrame()
	{
#if defined(_WIN32) && defined(_OPENGL)
		harray<hstr> renderSystems;
		renderSystems += APRIL_RS_OPENGL1;
		renderSystems += APRIL_RS_OPENGLES1;
		renderSystems += APRIL_RS_OPENGLES2;
		if (renderSystems.contains(april::rendersys->getName()))
		{
			SwapBuffers(((OpenGL_RenderSystem*)april::rendersys)->getHDC());
		}
#endif
	}
	
	Cursor* SDL_Window::_createCursor()
	{
		return new SDL_Cursor();
	}

	void SDL_Window::_handleSDLKeyEvent(Window::KeyEventType type, SDL_Keycode keysym, unsigned int unicode)
	{
		april::Key akeysym = AK_UNKNOWN;
	
		#define _s2a(sdlk, ak) case sdlk: akeysym = ak; break; 
		#define _s2a_u0(sdlk, ak) case sdlk: akeysym = ak; unicode = 0; break;
		#define s2a(sdlk, ak) _s2a(SDLK_ ## sdlk, AK_ ## ak)
		#define s2a_u0(sdlk, ak) _s2a_u0(SDLK_ ## sdlk, AK_ ## ak)
		#define sea(key) s2a(key, key)
		#define sea_u0(key) s2a_u0(key, key)
		
		switch (keysym)
		{
			// control character keys
			s2a(BACKSPACE, BACK);
#ifdef __APPLE__
			s2a_u0(DELETE, DELETE);
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
			sea_u0(LEFT);
			sea_u0(RIGHT);
			sea_u0(UP);
			sea_u0(DOWN);
			
			// space
			sea(SPACE);
			
			// function keys
			sea_u0(F1);
			sea_u0(F2);
			sea_u0(F3);
			sea_u0(F4);
			sea_u0(F5);
			sea_u0(F6);
			sea_u0(F7);
			sea_u0(F8);
			sea_u0(F9);
			sea_u0(F10);
			sea_u0(F11);
			sea_u0(F12);
			s2a(ESCAPE, ESCAPE);

			// keypad keys
			s2a(KP_0, NUMPAD0);
			s2a(KP_1, NUMPAD1);
			s2a(KP_2, NUMPAD2);
			s2a(KP_3, NUMPAD3);
			s2a(KP_4, NUMPAD4);
			s2a(KP_5, NUMPAD5);
			s2a(KP_6, NUMPAD6);
			s2a(KP_7, NUMPAD7);
			s2a(KP_8, NUMPAD8);
			s2a(KP_9, NUMPAD9);
			
			s2a(LCTRL, LCONTROL);
			s2a(RCTRL, RCONTROL);
			s2a(LALT, LMENU);
			s2a(RALT, RMENU);
			s2a(LGUI, LCOMMAND); // TODO - check if this is ok in SDL 2
			s2a(RGUI, RCOMMAND); // TODO - check if this is ok in SDL 2
			s2a(LSHIFT, LSHIFT);
			s2a(RSHIFT, RSHIFT);

		default:
			break;
		}
		// number keys
		if (keysym >= '0' && keysym <= '9')
		{
			akeysym = (Key)keysym;
		}
		// letters
		if (keysym >= 'a' && keysym <= 'z') // sdl letter keys are small ASCII letters
		{
			akeysym = (Key)(keysym - 32); // april letter keys are capital ASCII letters
		}
		if (akeysym == AK_LCONTROL || akeysym == AK_RCONTROL)
		{
			if (type == KEY_DOWN)
			{
				this->scrollHorizontal = true;
			}
			else if (type == KEY_UP)
			{
				this->scrollHorizontal = false;
			}
		}
		Window::handleKeyEvent(type, akeysym, 0);
	}
		
	void SDL_Window::_handleSDLMouseEvent(SDL_Event& sdlEvent)
	{
		this->cursorPosition.set((float)sdlEvent.button.x, (float)sdlEvent.button.y);
		Window::MouseEventType mouseEvent;
		april::Key mouseButton = AK_NONE;

		switch (sdlEvent.type)
		{
		case SDL_MOUSEBUTTONUP:
			mouseEvent = MOUSE_UP;
			break;
		case SDL_MOUSEBUTTONDOWN:
			mouseEvent = MOUSE_DOWN;
			break;
		case SDL_MOUSEMOTION:
			mouseEvent = MOUSE_MOVE;
			break;
		default:
			break;
		}

		if (sdlEvent.type == SDL_MOUSEBUTTONUP || sdlEvent.type == SDL_MOUSEBUTTONDOWN)
		{
			switch (sdlEvent.button.button)
			{
			case SDL_BUTTON_LEFT:
				mouseButton = AK_LBUTTON;
				break;
			case SDL_BUTTON_RIGHT:
				mouseButton = AK_RBUTTON;
				break;
			case SDL_BUTTON_MIDDLE:
				mouseButton = AK_MBUTTON;
				break;
			default:
				mouseButton = AK_NONE;
				break;
			}
		}

		if (sdlEvent.type == SDL_MOUSEWHEEL)
		{
			if (sdlEvent.wheel.x > 0 || sdlEvent.wheel.y > 0)
			{
				mouseButton = AK_WHEELDN;
				mouseEvent = MOUSE_SCROLL;
			}
			else if (sdlEvent.wheel.x < 0 || sdlEvent.wheel.y < 0)
			{
				mouseButton = AK_WHEELUP;
				mouseEvent = MOUSE_SCROLL;
			}
		}
		
		if (mouseButton == AK_WHEELUP || mouseButton == AK_WHEELDN)
		{
			gvec2 scroll;
			scroll.x = (!this->scrollHorizontal ? 0.0f : (mouseButton == AK_WHEELUP ? -1.0f : 1.0f));
			scroll.y = (this->scrollHorizontal ? 0.0f : (mouseButton == AK_WHEELUP ? -1.0f : 1.0f));
			this->handleMouseEvent(mouseEvent, scroll, mouseButton);
		}
		else
		{
			this->handleMouseEvent(mouseEvent, this->cursorPosition, mouseButton);
		}
	}
	
	float SDL_Window::_calcTimeSinceLastFrame()
	{
		static unsigned int x = SDL_GetTicks();
		float timeDelta = (SDL_GetTicks() - x) * 0.001f;
		x = SDL_GetTicks();
		if (timeDelta > 0.5f)
		{
			timeDelta = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
		}
		if (!this->focused)
		{
			timeDelta = 0.0f;
		}
		return timeDelta;
	}
		
}

#endif
