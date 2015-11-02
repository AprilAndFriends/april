/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _PVR
#include <april/Image.h>
#include <hltypes/hlog.h>
#include <hltypes/hstream.h>

#include "aprilpix.h"
#include "ImagePvr.h"
#include "PowerVR-SDK/PVRTGlobal.h"
#include "PowerVR-SDK/PVRTTexture.h"
#include "PowerVR-SDK/PVRTDecompress.h"

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
		int size = (int)stream.size();
		if (size < sizeof(PVRTextureHeaderV3))
		{
			return NULL;
		}
		uint8_t* data = new uint8_t[size];
		stream.readRaw(data, size);
		PVRTextureHeaderV3* header = (PVRTextureHeaderV3*)data;
		if (header->u32Width <= 0 || header->u32Height <= 0)
		{
			delete[] data;
			hlog::error(logTag, "Could not load PVR meta data!");
			return NULL;
		}
		if (header->u64PixelFormat != ePVRTPF_PVRTCI_4bpp_RGB && header->u64PixelFormat != ePVRTPF_PVRTCI_2bpp_RGB &&
			header->u64PixelFormat != ePVRTPF_PVRTCI_4bpp_RGBA && header->u64PixelFormat != ePVRTPF_PVRTCI_2bpp_RGBA)
		{
			hlog::error(logTag, "Unsupported pixel format!");
			return NULL;
		}
		april::Image* image = new ImagePvr();
		image->format = FORMAT_RGBA;
		image->w = header->u32Width;
		image->h = header->u32Height;
		image->data = new unsigned char[image->getByteSize()];
		int twoBitMode = (header->u64PixelFormat == ePVRTPF_PVRTCI_2bpp_RGB || header->u64PixelFormat == ePVRTPF_PVRTCI_2bpp_RGBA ? 1 : 0);
		int result = PVRTDecompressPVRTC(&data[sizeof(PVRTextureHeaderV3) + header->u32MetaDataSize], twoBitMode, image->w, image->h, image->data);
		if (result == 0)
		{
			hlog::warn(logTag, "PVR reported 0 bytes decompressed!");
		}
		delete[] data;
		return image;
	}

	april::Image* ImagePvr::loadMetaData(hsbase& stream)
	{
		int size = sizeof(PVRTextureHeaderV3);
		if ((int)stream.size() < size)
		{
			return NULL;
		}
		PVRTextureHeaderV3 header;
		stream.readRaw(&header, size);
		if (header.u32Width <= 0 || header.u32Height <= 0)
		{
			hlog::error(logTag, "Could not load PVR meta data!");
			return NULL;
		}
		if (header.u64PixelFormat != ePVRTPF_PVRTCI_4bpp_RGB && header.u64PixelFormat != ePVRTPF_PVRTCI_2bpp_RGB &&
			header.u64PixelFormat != ePVRTPF_PVRTCI_4bpp_RGBA && header.u64PixelFormat != ePVRTPF_PVRTCI_2bpp_RGBA)
		{
			hlog::error(logTag, "Unsupported pixel format!");
			return NULL;
		}
		april::Image* image = new ImagePvr();
		image->data = NULL;
		image->format = FORMAT_RGBA;
		image->w = header.u32Width;
		image->h = header.u32Height;
		return image;		
	}

}
#endif
