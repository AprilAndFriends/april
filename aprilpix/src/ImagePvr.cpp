/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "aprilpix.h"
#include "ImagePvr.h"

namespace aprilpix
{
	ImagePvr::ImagePvr() : april::Image()
	{
	}

	ImagePvr::~ImagePvr()
	{
	}

	april::Image* ImagePvr::load(hsbase& stream)
	{
		return NULL;
	}

	april::Image* ImagePvr::loadMetaData(hsbase& stream)
	{
		return NULL;
	}

}
