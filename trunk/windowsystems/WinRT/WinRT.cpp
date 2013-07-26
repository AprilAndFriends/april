/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "WinRT.h"

namespace april
{
	void (*WinRT::Init)(const harray<hstr>&) = NULL;
	void (*WinRT::Destroy)() = NULL;
	harray<hstr> WinRT::Args;
	WinRT_XamlApp^ WinRT::App = nullptr;
	WinRT_XamlInterface^ WinRT::Interface = nullptr;
	
}
#endif
