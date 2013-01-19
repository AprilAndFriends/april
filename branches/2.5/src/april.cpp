/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>
#ifdef USE_IL
#include <IL/il.h>
#endif
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
	#elif defined(_OPENGL)
	#define RS_INTERNAL_DEFAULT RS_OPENGL
	#else
	#warning "no rendersystems specified"
	#endif
	#if !_HL_WINRT
	#define WS_INTERNAL_DEFAULT WS_WIN32
	#else
	#define WS_INTERNAL_DEFAULT WS_WINRT
	#endif
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
	hstr logTag = "april";

	static harray<hstr> extensions;

	void log(chstr message, chstr prefix) // DEPRECATED
	{
		hlog::write(april::logTag, message);
	}
	
	void setLogFunction(void (*fnptr)(chstr)) // DEPRECATED
	{
	}

	void init(RenderSystemType renderSystemType, WindowSystemType windowSystemType)
	{
		hlog::write(april::logTag, "Initializing APRIL.");
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
#ifdef _DIRECTX11
		if (april::rendersys == NULL && renderSystem == RS_DIRECTX11)
		{
			april::rendersys = new DirectX11_RenderSystem();
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
#if !_HL_WINRT
		if (april::window == NULL && windowSystem == WS_WIN32)
		{
			april::window = new Win32_Window();
		}
#else
		if (april::window == NULL && windowSystem == WS_WINRT)
		{
			april::window = new WinRT_Window();
		}
#endif
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
			throw hl_exception("Could not create given rendersystem!");
		}
		if (april::window == NULL)
		{
			throw hl_exception("Could not create given windowsystem!");
		}
		hlog::writef(april::logTag, "Using: %s, %s", april::rendersys->getName().c_str(), april::window->getName().c_str());
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
	
	void destroy()
	{
		if (april::window != NULL)
		{
			delete april::window;
			april::window = NULL;
		}
		if (april::rendersys != NULL)
		{
			hlog::write(april::logTag, "Destroying APRIL.");
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
