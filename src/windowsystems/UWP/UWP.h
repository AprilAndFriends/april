/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines UWP utility and global stuff.

#ifdef _UWP
#ifndef APRIL_UWP_H
#define APRIL_UWP_H

#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "UWP_App.h"

using namespace Windows::Graphics::Display;

namespace april
{
	ref class UWP_App;

	class UWP
	{
	public:
		~UWP() { }
		
		static void (*Init)(const harray<hstr>&);
		static void (*Destroy)();
		static harray<hstr> Args;
		static UWP_App^ App;
		static float inline getDpiRatio() { return getDpiRatio(DisplayInformation::GetForCurrentView()->LogicalDpi); }
		static float inline getDpiRatio(float dpi) { return (dpi / 96.0f); }

	private:
		UWP() { }
		
	};
	
}

#endif
#endif
