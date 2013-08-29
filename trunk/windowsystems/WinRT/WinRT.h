/// @file
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
/// Defines a WinRT View.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_H
#define APRIL_WINRT_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#ifndef _WINP8
#include "WinRT_XamlApp.h"
#include "WinRT_XamlInterface.xaml.h"
#else
#include "WinRT_View.h"
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
#ifndef _WINP8
		static WinRT_XamlApp^ App;
		static WinRT_XamlInterface^ Interface;
#else
		static WinRT_View^ View;
#endif
		
	private:
		WinRT() { }
		
	};
	
}

#endif
#endif
