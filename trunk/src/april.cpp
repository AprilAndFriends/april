/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>
#ifdef _ANDROID
#include <android/log.h>
#endif
#ifdef USE_IL
#include <IL/il.h>
#endif
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#ifdef _DIRECTX9
#include "DirectX9_RenderSystem.h"
#endif
#ifdef _OPENGL
#include "OpenGL_RenderSystem.h"
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
#include "Win32_Window.h"
#endif

#ifdef _WIN32
	#ifdef _DIRECTX9
	#define RS_INTERNAL_DEFAULT RS_DIRECTX9
	#elif defined(_OPENGL)
	#define RS_INTERNAL_DEFAULT RS_OPENGL
	#else
	#warning "no rendersystems specified"
	#endif
	#define WS_INTERNAL_DEFAULT WS_WIN32
#elif defined(__APPLE__) && !TARGET_OS_IPHONE
	#define RS_INTERNAL_DEFAULT RS_OPENGL
	#define WS_INTERNAL_DEFAULT WS_SDL
	#ifndef HAVE_SDL
	#warning "no windowsystems specified"
	#endif
#elif defined(__APPLE__) && TARGET_OS_IPHONE
	#define RS_INTERNAL_DEFAULT RS_OPENGL
	#define WS_INTERNAL_DEFAULT WS_IOS
	#if !TARGET_OS_IPHONE
	#warning "no windowsystems specified"
	#endif
#elif defined(_UNIX)
	#define RS_INTERNAL_DEFAULT RS_OPENGL
	#define WS_INTERNAL_DEFAULT WS_SDL
	#ifndef HAVE_SDL
	#warning "no windowsystems specified"
	#endif
#elif defined(_ANDROID)
	#define RS_INTERNAL_DEFAULT RS_OPENGL
	#define WS_INTERNAL_DEFAULT WS_ANDROIDJNI
#else
	#warning "no platform specified"
#endif


namespace april
{
	harray<hstr> extensions;
	hstr systemPath = ".";

	void april_writelog(chstr message)
	{
#ifndef _ANDROID
		printf("%s\n", message.c_str());
#else
		__android_log_print(ANDROID_LOG_INFO, "april", "%s", message.c_str());
#endif
	}
	
	void (*g_logFunction)(chstr) = april_writelog;
	
	void log(chstr message, chstr prefix)
	{
		g_logFunction(prefix + message);
	}
	
	void init(RenderSystemType renderSystemType, WindowSystemType windowSystemType)
	{
		april::log("initializing april");
#ifdef USE_IL
		ilInit();
#endif
		extensions += ".jpt";
		extensions += ".png";
		extensions += ".jpg";
#if TARGET_OS_IPHONE
		extensions += ".pvr";
#endif
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
#ifdef _OPENGL
		if (april::rendersys == NULL && renderSystem == RS_OPENGL)
		{
			april::rendersys = new OpenGL_RenderSystem();
		}
#endif
		// creating the windowsystem
		WindowSystemType windowSystem = windowSystemType;
		if (windowSystem == WS_DEFAULT)
		{
			windowSystem = WS_INTERNAL_DEFAULT;
		}
#ifdef _WIN32
		if (april::window == NULL && windowSystem == WS_WIN32)
		{
			april::window = new Win32_Window();
		}
#endif
#ifdef HAVE_SDL
		if (april::window == NULL && windowSystem == WS_SDL)
		{
			april::window = new SDL_Window();
		}
#endif
#if TARGET_OS_IPHONE
		if (april::window == NULL && windowSystem == WS_IOS)
		{
			april::window = new iOS_Window();
		}
#endif
#ifdef _ANDROID
		if (april::window == NULL && windowSystem == WS_ANDROIDJNI)
		{
			april::window = new AndroidJNI_Window();
		}
#endif
		if (april::rendersys == NULL)
		{
			throw hl_exception("could not create given rendersystem");
		}
		if (april::window == NULL)
		{
			throw hl_exception("could not create given windowsystem");
		}
		april::log(hsprintf("using: %s, %s", april::rendersys->getName().c_str(), april::window->getName().c_str()));
	}
	
	void createRenderSystem(chstr options)
	{
		april::rendersys->create(options);
	}
	
	void createWindow(int w, int h, bool fullscreen, chstr title)
	{
		april::window->create(w, h, fullscreen, title);
		april::rendersys->assignWindow(april::window);
	}

	void init(RenderSystemType renderSystemType, WindowSystemType windowSystemType, chstr renderSystemOptions, int w, int h, bool fullscreen, chstr title)
	{
		init(renderSystemType, windowSystemType);
		createRenderSystem(renderSystemOptions);
		createWindow(w, h, fullscreen, title);
	}
	
	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction = fnptr;
	}
	
	void destroy()
	{
		if (april::window != NULL)
		{
			delete april::window;
			april::window = NULL;
		}
		if (april::rendersys != NULL)
		{
			april::log("destroying april");
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
