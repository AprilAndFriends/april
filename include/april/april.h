/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.42
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

#define APRIL_RS_OPENGL "OpenGL"
#define APRIL_RS_DIRECTX9 "DirectX9"
#define APRIL_RS_DEFAULT ""

#define APRIL_WS_WIN32 "Win32"
#define APRIL_WS_WINRT "WinRT"
#define APRIL_WS_SDL "SDL"
#define APRIL_WS_ANDROIDJNI "AndroidJNI"
#define APRIL_WS_IOS "iOS"
#define APRIL_WS_DEFAULT ""

namespace april
{
	extern hstr logTag;

	enum RenderSystemType
	{
		RS_DEFAULT = 0,
		RS_DIRECTX9 = 1,
		RS_OPENGL = 2
	};

	enum WindowSystemType
	{
		WS_DEFAULT = 0,
		WS_WIN32 = 1,
		WS_WINRT = 2,
		WS_SDL = 3,
		WS_IOS = 4,
		WS_ANDROIDJNI = 5
	};

	extern hstr systemPath;

	aprilFnExport void init(RenderSystemType renderSystemType, WindowSystemType windowSystemType);
	aprilFnExport void init(RenderSystemType renderSystemType, WindowSystemType windowSystemType, chstr renderSystemOptions, int w, int h, bool fullscreen, chstr title);
	aprilFnExport void createRenderSystem(chstr options = "");
	aprilFnExport void createWindow(int w, int h, bool fullscreen, chstr title);
	aprilFnExport void destroy();
	aprilFnExport void addTextureExtension(chstr extension);
	aprilFnExport harray<hstr> getTextureExtensions();
	aprilFnExport void setTextureExtensions(const harray<hstr>& exts);

	DEPRECATED_ATTRIBUTE aprilFnExport void setLogFunction(void (*fnptr)(chstr));
	DEPRECATED_ATTRIBUTE aprilFnExport void log(chstr message, chstr prefix = "[april] ");

}

#endif
