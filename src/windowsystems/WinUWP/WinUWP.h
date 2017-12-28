/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines WinUWP utility and global stuff.

#ifdef _WINUWP
#ifndef APRIL_WINUWP_H
#define APRIL_WINUWP_H

#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "WinUWP_App.h"

using namespace Windows::Graphics::Display;

namespace april
{
	ref class WinUWP_App;

	class WinUWP
	{
	public:
		~WinUWP() { }
		
		static void (*Init)(const harray<hstr>&);
		static void (*Destroy)();
		static harray<hstr> Args;
		static WinUWP_App^ App;
		static float inline getDpiRatio() { return getDpiRatio(DisplayInformation::GetForCurrentView()->LogicalDpi); }
		static float inline getDpiRatio(float dpi) { return (dpi / 96.0f); }

	private:
		WinUWP() { }
		
	};
	
}

#endif
#endif
