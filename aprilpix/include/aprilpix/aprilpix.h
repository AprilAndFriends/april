/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines all functions for April PIX.

#ifndef APRILPIX_H
#define APRILPIX_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilpixExport.h"

namespace aprilpix
{
	extern hstr logTag;

	aprilpixFnExport void init();
	aprilpixFnExport void destroy();
	aprilpixFnExport harray<hstr> getExtensions();

};

#endif

