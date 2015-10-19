/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/april.h>
#include <april/Image.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "aprilpix.h"
#include "ImagePvr.h"
#include "ImageWebp.h"

namespace aprilpix
{
	hstr logTag = "aprilpix";

	void init()
	{
		hlog::write(logTag, "Initializing AprilPIX");
#ifdef _WEBP
		harray<hstr> extensions = april::getTextureExtensions();
		extensions += ".webp";
		april::setTextureExtensions(extensions);
		april::Image::registerCustomLoader(".webp", &ImageWebp::load, &ImageWebp::loadMetaData);
#endif
	}

	void destroy()
	{
		hlog::write(logTag, "Destroying AprilPix");
	}

}
