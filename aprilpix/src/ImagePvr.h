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
/// Defines methods for loading a PVR image.

#ifdef _PVR
#ifndef APRILPIX_IMAGE_PVR_H
#define APRILPIX_IMAGE_PVR_H

#include <april/Image.h>
#include <hltypes/harray.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "Tools/PVRTTexture.h"

#define PVR_HEADER_SIZE 52

namespace aprilpix
{	

	class ImagePvr : public april::Image
	{	
	public:
		~ImagePvr();

		static PVRTextureHeaderV3 pvrGetInfo(uint8_t* data, int size, int* width, int* height);

		static april::Image* load(hsbase& stream);
		static april::Image* loadMetaData(hsbase& stream);
	
	protected:
		ImagePvr();

	};

};
#endif
#endif
