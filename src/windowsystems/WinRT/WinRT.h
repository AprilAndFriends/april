/// @file
/// @version 3.5
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

#include "WinRT_XamlApp.h"

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

	private:
		WinRT() { }
		
	};
	
}

#endif
#endif
