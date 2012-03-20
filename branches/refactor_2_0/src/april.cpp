/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 1.51
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

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX9_RenderSystem.h"
#include "OpenGL_RenderSystem.h"
#include "RenderSystem.h"
#include "SDL_Window.h"
#include "Win32_Window.h"
#include "Window.h"

namespace april
{
	DEPRECATED_ATTRIBUTE void createRenderTarget(int width, int height, bool fullscreen, chstr title) { createWindow(width, height, fullscreen, title); } // DEPRECATED

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
	
	void init()
	{
#ifdef USE_IL
		ilInit();
#endif
		extensions += ".png";
		extensions += ".jpg";
#if TARGET_OS_IPHONE
		extensions += ".pvr";
#endif
	}
	
	void createRenderSystem(chstr options)
	{
#if TARGET_OS_IPHONE
		if (options == "create_eagl")
		{
			april::rendersys = OpenGL_RenderSystem::create(options);
		}
		else
		{
			april::rendersys->setParam(options, "");
		}
		//else do nothing, rendersys was created ahead
#elif defined(_OPENGL)
		april::rendersys = OpenGL_RenderSystem::create(options);
#else
		april::rendersys = DirectX9_RenderSystem::create(options);
#endif
	}
	
	void createWindow(int width, int height, bool fullscreen, chstr title)
	{
#if TARGET_OS_IPHONE
		return;
#elif defined(_WIN32)
#ifndef HAVE_SDL
		april::window = new Win32_Window(width, height, fullscreen, title);
#else
		april::window = new SDL_Window(width, height, fullscreen, title);
#endif
#elif defined(_ANDROID)
		april::window = new AndroidJNI_Window(width, height, fullscreen, title);
#else
		april::window = new SDL_Window(width, height, fullscreen, title);
#endif
		april::rendersys->assignWindow(april::window);
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
