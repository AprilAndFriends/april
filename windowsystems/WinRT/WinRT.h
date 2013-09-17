/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines WinRT utility and global stuff.

#ifdef _WINRT
#ifndef APRIL_WINRT_H
#define APRIL_WINRT_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "WinRT_BaseApp.h"
#ifndef _WINP8
#include "WinRT_XamlOverlay.xaml.h"
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
#endif

	private:
		WinRT() { }
		
	};
	
}

#endif
#endif
