/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic render system.

#ifndef APRIL_H
#define APRIL_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>

#include "aprilExport.h"

#define APRIL_RS_OPENGL1 "OpenGL1"
#define APRIL_RS_OPENGLES1 "OpenGLES1"
#define APRIL_RS_DIRECTX9 "DirectX9"
#define APRIL_RS_DIRECTX11 "DirectX11"
#define APRIL_RS_DEFAULT ""

#define APRIL_WS_WIN32 "Win32"
#define APRIL_WS_WINRT "WinRT"
#define APRIL_WS_SDL "SDL"
#define APRIL_WS_ANDROIDJNI "AndroidJNI"
#define APRIL_WS_IOS "iOS"
#define APRIL_WS_DEFAULT ""

namespace april
{
	class RenderSystem;
	class Window;

	extern hstr logTag;

	enum RenderSystemType
	{
		RS_DEFAULT = 0,
		RS_DIRECTX9 = 1,
		RS_DIRECTX11 = 2,
		RS_OPENGL1 = 3,
		RS_OPENGLES1 = 4,
		RS_CUSTOM = 5
	};

	enum WindowType
	{
		WS_DEFAULT = 0,
		WS_WIN32 = 1,
		WS_WINRT = 2,
		WS_SDL = 3,
		WS_IOS = 4,
		WS_ANDROIDJNI = 5
	};

	aprilFnExport void init(RenderSystemType renderSystemType, WindowType windowType);
	aprilFnExport void init(RenderSystem* customRenderSystem, WindowType windowType);
	aprilFnExport void init(RenderSystemType renderSystemType, Window* customWindow);
	aprilFnExport void init(RenderSystem* customRenderSystem, Window* customWindow);
	aprilFnExport void init(RenderSystemType renderSystemType, WindowType windowType,
		chstr renderSystemOptions, int w, int h, bool fullscreen, chstr title, chstr windowOptions);
	aprilFnExport void init(RenderSystem* customRenderSystem, WindowType windowType,
		chstr renderSystemOptions, int w, int h, bool fullscreen, chstr title, chstr windowOptions);
	aprilFnExport void init(RenderSystemType renderSystemType, Window* customWindow,
		chstr renderSystemOptions, int w, int h, bool fullscreen, chstr title, chstr windowOptions);
	aprilFnExport void init(RenderSystem* customRenderSystem, Window* customWindow,
		chstr renderSystemOptions, int w, int h, bool fullscreen, chstr title, chstr windowOptions);
	aprilFnExport void createRenderSystem(chstr options = "");
	aprilFnExport void createWindow(int w, int h, bool fullscreen, chstr title, chstr options = "");
	aprilFnExport void destroy();
	aprilFnExport void addTextureExtension(chstr extension);
	aprilFnExport harray<hstr> getTextureExtensions();
	aprilFnExport void setTextureExtensions(const harray<hstr>& exts);

}

#endif
