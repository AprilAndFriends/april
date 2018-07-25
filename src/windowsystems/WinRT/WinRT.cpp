/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT_WINDOW
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "WinRT.h"

namespace april
{
	void (*WinRT::Init)(const harray<hstr>&) = NULL;
	void (*WinRT::Destroy)() = NULL;
	WinRT_XamlApp^ WinRT::App = nullptr;

}
#endif
