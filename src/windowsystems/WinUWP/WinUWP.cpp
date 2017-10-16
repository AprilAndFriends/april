/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINUWP
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "WinUWP.h"

namespace april
{
	void (*WinUWP::Init)(const harray<hstr>&) = NULL;
	void (*WinUWP::Destroy)() = NULL;
	harray<hstr> WinUWP::Args;
	WinUWP_App^ WinUWP::App = nullptr;

}
#endif
