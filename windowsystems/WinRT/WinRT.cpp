/// @file
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "IWinRT.h"
#include "WinRT.h"

#ifdef _WINP8
using namespace Windows::Graphics::Display;
#endif

namespace april
{
	void (*WinRT::Init)(const harray<hstr>&) = NULL;
	void (*WinRT::Destroy)() = NULL;
	harray<hstr> WinRT::Args;
	IWinRT^ WinRT::Interface = nullptr;

#ifndef _WINP8
	WinRT_XamlOverlay^ WinRT::XamlOverlay = nullptr;
#else
	int WinRT::getScreenRotation()
	{
		switch (DisplayProperties::CurrentOrientation)
		{
		case DisplayOrientations::Portrait:			return 0;
		case DisplayOrientations::LandscapeFlipped:	return 270;
		case DisplayOrientations::PortraitFlipped:	return 180;
		}
		return 90; // default is landscape
	}
#endif
	
}
#endif
