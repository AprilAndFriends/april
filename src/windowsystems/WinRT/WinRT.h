/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines WinRT utility and global stuff.

#ifdef _WINRT
#ifndef APRIL_WINRT_H
#define APRIL_WINRT_H

#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "IWinRT.h"
#include "WinRT_BaseApp.h"
#ifndef _WINP8
#include "WinRT_XamlOverlay.xaml.h"
#endif

#ifndef _WINP8
#define CHECK_SWAP(w, h)
#else
using namespace Windows::Graphics::Display;
#define CHECK_SWAP(w, h) \
	if (DisplayProperties::NativeOrientation == DisplayOrientations::Portrait || \
		DisplayProperties::NativeOrientation == DisplayOrientations::PortraitFlipped) \
	{ \
		hswap(w, h); \
	}
#endif

namespace april
{
	class WinRT
	{
	public:
		~WinRT() { }
		
		static void (*Init)(const harray<hstr>&);
		static void (*Destroy)();
		static harray<hstr> Args;
		static IWinRT^ Interface;
#ifndef _WINP8
		static WinRT_XamlOverlay^ XamlOverlay;
#else
		static int getScreenRotation();
		static grect rotateViewport(grect viewport);
#endif

	private:
		WinRT() { }
		
	};
	
}

#endif
#endif
