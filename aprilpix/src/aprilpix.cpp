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
#ifdef _PVR
#include "ImagePvr.h"
#endif
#ifdef _WEBP
#include "ImageWebp.h"
#endif

namespace aprilpix
{
	hstr logTag = "aprilpix";

	void init()
	{
		hlog::write(logTag, "Initializing AprilPIX");
		harray<hstr> extensions = april::getTextureExtensions();
#ifdef _WEBP
		extensions += ".webp";
		april::Image::registerCustomLoader(".webp", &ImageWebp::load, &ImageWebp::loadMetaData);
#endif
#ifdef _PVR
		extensions += ".pvr";
		april::Image::registerCustomLoader(".pvr", &ImagePvr::load, &ImagePvr::loadMetaData);
#endif
		april::setTextureExtensions(extensions);
	}

	void destroy()
	{
		hlog::write(logTag, "Destroying AprilPix");
	}

}
