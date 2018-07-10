/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/april.h>
#include <april/Image.h>
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#include <hltypes/hversion.h>

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
	static hversion version(1, 1, 0);

	void init()
	{
		hlog::write(logTag, "Initializing AprilPIX: " + version.toString());
#ifdef _WEBP
		april::Image::registerCustomLoader(".webp", &ImageWebp::load, &ImageWebp::loadMetaData);
		april::Image::registerCustomSaver(".webp", &ImageWebp::save, &ImageWebp::makeDefaultSaveParameters);
#endif
#ifdef _PVR
		april::Image::registerCustomLoader(".pvr", &ImagePvr::load, &ImagePvr::loadMetaData);
		//april::Image::registerCustomSaver(".pvr", &ImagePvr::save);
#endif
	}

	void destroy()
	{
		hlog::write(logTag, "Destroying AprilPIX.");
	}

	harray<hstr> getExtensions()
	{
		harray<hstr> extensions;
#ifdef _WEBP
		extensions += ".webp";
#endif
#ifdef _PVR
		extensions += ".pvr";
#endif
		return extensions;
	}

}
