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
		if (size < sizeof(PVR_Texture_Header))
		{
			hlog::error(logTag, "PVR v1 not supported!");
			return NULL;
		}
		uint8_t* data = new uint8_t[size];
		stream.readRaw(data, size);
		PVR_Texture_Header* header = (PVR_Texture_Header*)data;
		if (header->dwWidth <= 0 || header->dwHeight <= 0)
		{
			delete[] data;
			hlog::error(logTag, "Could not load PVR meta data!");
			return NULL;
		}
		if (header->dwBitCount != 4 && header->dwBitCount != 2 && header->dwpfFlags != MGLPT_PVRTC4 && header->dwpfFlags != OGL_PVRTC4)
		{
			hlog::error(logTag, "Unsupported pixel format!");
			return NULL;
		}
		april::Image* image = new ImagePvr();
		image->format = FORMAT_RGBA;
		image->w = header->dwWidth;
		image->h = header->dwHeight;
		image->data = new unsigned char[image->getByteSize()];
		int twoBitMode = (header->dwBitCount == 2);
		int result = PVRTDecompressPVRTC(&data[sizeof(PVR_Texture_Header)], twoBitMode, image->w, image->h, image->data);
		if (result == 0)
		{
			hlog::warn(logTag, "PVR reported 0 bytes decompressed!");
		}
		delete[] data;
		return image;
	}

	april::Image* ImagePvr::loadMetaData(hsbase& stream)
	{
		int size = sizeof(PVR_Texture_Header);
		if ((int)stream.size() < size)
		{
			return NULL;
		}
		PVR_Texture_Header header;
		stream.readRaw(&header, size);
		if (header.dwWidth <= 0 || header.dwHeight <= 0)
		{
			hlog::error(logTag, "Could not load PVR meta data!");
			return NULL;
		}
		if (header.dwBitCount != 4 && header.dwBitCount != 2 && header.dwpfFlags != MGLPT_PVRTC4 && header.dwpfFlags != OGL_PVRTC4)
		{
			hlog::error(logTag, "Unsupported pixel format!");
			return NULL;
		}
		april::Image* image = new ImagePvr();
		image->data = NULL;
		image->format = FORMAT_RGBA;
		image->w = header.dwWidth;
		image->h = header.dwHeight;
		return image;		
	}

}
#endif
