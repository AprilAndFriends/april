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

#include "Tools/PVRTGlobal.h"
#include "Tools/PVRTTexture.h"
#include "Tools/PVRTDecompress.h"

namespace aprilpix
{	
	#define PVR_HEADER_SIZE 52

	/*struct PVRHeader
	{
		uint32_t	u32Version;
		uint32_t	u32Flags;	
		uint64_t	u64PixelFormat;
		uint32_t	u32ColourSpace;
		uint32_t	u32ChannelType;
		uint32_t	u32Height;		
		uint32_t	u32Width;		
		uint32_t	u32Depth;		
		uint32_t	u32NumSurfaces;	
		uint32_t	u32NumFaces;	
		uint32_t	u32MIPMapCount;	
		uint32_t	u32MetaDataSize;
	};
	enum PVRPixelFormat
	{
		ePVRTPF_PVRTCI_2bpp_RGB,
		ePVRTPF_PVRTCI_2bpp_RGBA,
		ePVRTPF_PVRTCI_4bpp_RGB,
		ePVRTPF_PVRTCI_4bpp_RGBA
	};*/

	typedef PVRTextureHeaderV3 PVRHeader;
	typedef EPVRTPixelFormat PVRPixelFormat;

	class ImagePvr : public april::Image
	{	
	public:
		~ImagePvr();

		static PVRHeader pvrGetInfo(uint8_t* data, int size, int* width, int* height);

		static april::Image* load(hsbase& stream);
		static april::Image* loadMetaData(hsbase& stream);
	
	protected:
		ImagePvr();

	};

};
#endif
#endif
