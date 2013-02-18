/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#ifdef _DIRECTX9
#include "DirectX9_RenderSystem.h"
#endif
#ifdef _DIRECTX11
#include "DirectX11_RenderSystem.h"
#endif
#ifdef _OPENGL1
#include "OpenGL1_RenderSystem.h"
#endif
#ifdef _OPENGLES1
#include "OpenGLES1_RenderSystem.h"
#endif
#ifdef _OPENGLES2
#include "OpenGLES2_RenderSystem.h"
#endif
#include "RenderSystem.h"
#include "Window.h"
#ifdef _ANDROID
#include "AndroidJNI_Window.h"
#endif
#if TARGET_OS_IPHONE
#include "iOS_Window.h"
#endif
#ifdef HAVE_SDL
#include "SDL_Window.h"
#endif
#ifdef _WIN32
#if !_HL_WINRT
#include "Win32_Window.h"
#else
#include "WinRT_Window.h"
#endif
#endif

#ifdef _WIN32
	#ifdef _DIRECTX9
		#define RS_INTERNAL_DEFAULT RS_DIRECTX9
	#elif defined(_DIRECTX11)
		#define RS_INTERNAL_DEFAULT RS_DIRECTX11
	#elif defined(_OPENGL1)
		#define RS_INTERNAL_DEFAULT RS_OPENGL1
	#elif defined(_OPENGLES1)
		#define RS_INTERNAL_DEFAULT RS_OPENGLES1
	#elif defined(_OPENGLES2)
		#define RS_INTERNAL_DEFAULT RS_OPENGLES2
	#endif
	#if !_HL_WINRT
		#define WS_INTERNAL_DEFAULT WS_WIN32
	#else
		#define WS_INTERNAL_DEFAULT WS_WINRT
	#endif
#elif defined(__APPLE__) && !TARGET_OS_IPHONE
	#ifdef _OPENGLES2
		#define RS_INTERNAL_DEFAULT RS_OPENGLES2
	#elif defined(_OPENGLES1)
		#define RS_INTERNAL_DEFAULT RS_OPENGLES1
	#endif
	#define WS_INTERNAL_DEFAULT WS_SDL
	#ifndef HAVE_SDL
		#define RS_INTERNAL_DEFAULT RS_DEFAULT
	#endif
#elif defined(__APPLE__) && TARGET_OS_IPHONE
	#ifdef _OPENGLES2
		#define RS_INTERNAL_DEFAULT RS_OPENGLES2
	#elif defined(_OPENGLES1)
		#define RS_INTERNAL_DEFAULT RS_OPENGLES1
	#endif
	#define WS_INTERNAL_DEFAULT WS_IOS
#elif defined(_UNIX)
	#define RS_INTERNAL_DEFAULT RS_OPENGL1
	#define WS_INTERNAL_DEFAULT WS_SDL
#elif defined(_ANDROID)
	#ifdef _OPENGLES1
		#define RS_INTERNAL_DEFAULT RS_OPENGLES1
	#elif defined(_OPENGLES2)
		#define RS_INTERNAL_DEFAULT RS_OPENGLES2
	#endif
	#define WS_INTERNAL_DEFAULT WS_ANDROIDJNI
#endif

#ifndef RS_INTERNAL_DEFAULT
#define RS_INTERNAL_DEFAULT RS_DEFAULT
#endif
#ifndef WS_INTERNAL_DEFAULT
#define WS_INTERNAL_DEFAULT WS_DEFAULT
#endif

namespace april
{
	hstr logTag = "april";

	static harray<hstr> extensions;

	void _startInit()
	{
		hlog::write(april::logTag, "Initializing APRIL.");
		extensions += ".jpt";
		extensions += ".png";
		extensions += ".jpg";
#if TARGET_OS_IPHONE
		extensions += ".pvr";
#endif
	}

	void _finishInit()
	{
		hlog::writef(april::logTag, "Using: %s, %s", april::rendersys->getName().c_str(), april::window->getName().c_str());
	}

	void _createRenderSystem(RenderSystemType renderSystemType)
	{
		// creating the rendersystem
		RenderSystemType renderSystem = renderSystemType;
		if (renderSystem == RS_DEFAULT)
		{
			renderSystem = RS_INTERNAL_DEFAULT;
		}
#ifdef _DIRECTX9
		if (april::rendersys == NULL && renderSystem == RS_DIRECTX9)
		{
			april::rendersys = new DirectX9_RenderSystem();
		}
#endif
#ifdef _DIRECTX11
		if (april::rendersys == NULL && renderSystem == RS_DIRECTX11)
		{
			april::rendersys = new DirectX11_RenderSystem();
		}
#endif
#ifdef _OPENGL1
		if (april::rendersys == NULL && renderSystem == RS_OPENGL1)
		{
			april::rendersys = new OpenGL1_RenderSystem();
		}
#endif
#ifdef _OPENGLES1
		if (april::rendersys == NULL && renderSystem == RS_OPENGLES1)
		{
			april::rendersys = new OpenGLES1_RenderSystem();
		}
#endif
#ifdef _OPENGLES2
		if (april::rendersys == NULL && renderSystem == RS_OPENGLES2)
		{
			april::rendersys = new OpenGLES2_RenderSystem();
		}
#endif
		if (april::rendersys == NULL)
		{
			throw hl_exception("Could not create given rendersystem!");
		}
	}

	void _createWindowSystem(WindowType windowType)
	{
		// creating the windowsystem
		WindowType window = windowType;
		if (window == WS_DEFAULT)
		{
			window = WS_INTERNAL_DEFAULT;
		}
#ifdef HAVE_WIN32
		if (april::window == NULL && window == WS_WIN32)
		{
			april::window = new Win32_Window();
		}
#endif
#ifdef HAVE_WINRT
		if (april::window == NULL && window == WS_WINRT)
		{
			april::window = new WinRT_Window();
		}
#endif
#ifdef HAVE_SDL
		if (april::window == NULL && window == WS_SDL)
		{
			april::window = new SDL_Window();
		}
#endif
#if TARGET_OS_IPHONE
		if (april::window == NULL && window == WS_IOS)
		{
			april::window = new iOS_Window();
		}
#endif
#ifdef _ANDROID
		if (april::window == NULL && window == WS_ANDROIDJNI)
		{
			april::window = new AndroidJNI_Window();
		}
#endif
		if (april::window == NULL)
		{
			throw hl_exception("Could not create given windowsystem!");
		}
	}

	void init(RenderSystemType renderSystemType, WindowType windowType)
	{
		_startInit();
		_createRenderSystem(renderSystemType);
		_createWindowSystem(windowType);
		_finishInit();
	}
	
	void init(RenderSystem* customRenderSystem, WindowType windowType)
	{
		_startInit();
		april::rendersys = customRenderSystem;
		_createWindowSystem(windowType);
		_finishInit();
	}
	
	void init(RenderSystemType renderSystemType, Window* customWindow)
	{
		_startInit();
		_createRenderSystem(renderSystemType);
		april::window = customWindow;
		_finishInit();
	}
	
	void init(RenderSystem* customRenderSystem, Window* customWindow)
	{
		_startInit();
		april::rendersys = customRenderSystem;
		april::window = customWindow;
		_finishInit();
	}
	
	void init(RenderSystemType renderSystemType, WindowType windowType, chstr renderSystemOptions,
		int w, int h, bool fullscreen, chstr title, chstr windowOptions)
	{
		init(renderSystemType, windowType);
		createRenderSystem(renderSystemOptions);
		createWindow(w, h, fullscreen, title, windowOptions);
	}
	
	void init(RenderSystem* customRenderSystem, WindowType windowType, chstr renderSystemOptions,
		int w, int h, bool fullscreen, chstr title, chstr windowOptions)
	{
		init(customRenderSystem, windowType);
		createRenderSystem(renderSystemOptions);
		createWindow(w, h, fullscreen, title, windowOptions);
	}
	
	void init(RenderSystemType renderSystemType, Window* customWindow, chstr renderSystemOptions,
		int w, int h, bool fullscreen, chstr title, chstr windowOptions)
	{
		init(renderSystemType, customWindow);
		createRenderSystem(renderSystemOptions);
		createWindow(w, h, fullscreen, title, windowOptions);
	}
	
	void init(RenderSystem* customRenderSystem, Window* customWindow, chstr renderSystemOptions,
		int w, int h, bool fullscreen, chstr title, chstr windowOptions)
	{
		init(customRenderSystem, customWindow);
		createRenderSystem(renderSystemOptions);
		createWindow(w, h, fullscreen, title, windowOptions);
	}
	
	void createRenderSystem(chstr options)
	{
		april::rendersys->create(options);
	}
	
	void createWindow(int w, int h, bool fullscreen, chstr title, chstr options)
	{
		april::window->create(w, h, fullscreen, title, options);
		april::rendersys->assignWindow(april::window);
	}

	void destroy()
	{
		if (april::rendersys != NULL || april::window != NULL)
		{
			hlog::write(april::logTag, "Destroying APRIL.");
		}
		if (april::window != NULL)
		{
			delete april::window;
			april::window = NULL;
		}
		if (april::rendersys != NULL)
		{
			delete april::rendersys;
			april::rendersys = NULL;
		}
	}
	
	void addTextureExtension(chstr extension)
	{
		extensions += extension;
	}
	
	harray<hstr> getTextureExtensions()
	{
		return extensions;
	}
	
	void setTextureExtensions(const harray<hstr>& exts)
	{
		extensions = exts;
	}

}
