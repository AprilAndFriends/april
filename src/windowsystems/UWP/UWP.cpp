/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UWP
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "UWP.h"

namespace april
{
	void (*UWP::Init)(const harray<hstr>&) = NULL;
	void (*UWP::Destroy)() = NULL;
	harray<hstr> UWP::Args;
	UWP_App^ UWP::App = nullptr;

}
#endif
