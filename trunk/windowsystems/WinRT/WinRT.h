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
#ifndef APRIL_WINRT_VIEW_H
#define APRIL_WINRT_VIEW_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "WinRT_XamlApp.h"
#include "WinRT_XamlInterface.xaml.h"

namespace april
{
	class WinRT
	{
	public:
		~WinRT() { }
		
		static void (*Init)(const harray<hstr>&);
		static void (*Destroy)();
		static harray<hstr> Args;
		static WinRT_XamlApp^ App;
		static WinRT_XamlInterface^ Interface;
		
	private:
		WinRT() { }
		
	};
	
}

#endif
#endif
