/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "aprilpix.h"
#include "ImageWebp.h"

namespace aprilpix
{
	ImageWebp::ImageWebp() : april::Image()
	{
	}

	ImageWebp::~ImageWebp()
	{
	}

	april::Image* ImageWebp::load(hsbase& stream)
	{
		return NULL;
	}

	april::Image* ImageWebp::loadMetaData(hsbase& stream)
	{
		return NULL;
	}

}
