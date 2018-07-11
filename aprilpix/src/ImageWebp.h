/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines methods for loading a WEBP image.

#ifdef _WEBP
#ifndef APRILPIX_IMAGE_WEBP_H
#define APRILPIX_IMAGE_WEBP_H

#include <april/Image.h>
#include <hltypes/harray.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

namespace aprilpix
{
	class ImageWebp : public april::Image
	{
	public:
		~ImageWebp();

		static april::Image* load(hsbase& stream);
		static april::Image* loadMetaData(hsbase& stream);
#ifndef _WEBP_NO_ENCODE
		static bool save(hsbase& stream, april::Image* image, april::Image::SaveParameters parameters);
		static april::Image::SaveParameters makeDefaultSaveParameters();
#endif

	protected:
		ImageWebp();

	};

};
#endif
#endif
