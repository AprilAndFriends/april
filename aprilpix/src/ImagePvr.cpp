/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _PVR
#include <algorithm>
#include <iostream>

#include <april/Image.h>
#include <hltypes/hlog.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

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

	
	PVRHeader ImagePvr::pvrGetInfo(uint8_t* data, int size, int* width, int* height)
	{
		PVRHeader header;
		memcpy(&header, data, sizeof(PVRHeader));
		*width = header.u32Width;
		*height = header.u32Height;		

		return header;
	}
	

	april::Image* ImagePvr::load(hsbase& stream)
	{		
		int size = (int)stream.size();
		uint8_t* data = new uint8_t[size];
		stream.readRaw(data, size);
		int width = 0;
		int height = 0;
		PVRHeader header = pvrGetInfo(data, size, &width, &height);
		
		if (width <= 0 || height <= 0)
		{
			hlog::error(logTag, "Could not load PVR meta data!");
			return NULL;
		}
		int bpp = 4;
		april::Image* image = new ImagePvr();
		if (header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGB)
		{
			image->format = FORMAT_RGB;
			bpp = 4;
		}
		else if (header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGBA)		
		{
			image->format = FORMAT_RGBA;
			bpp = 4;
		}
		
		int dataOffset = sizeof(PVRHeader) + header.u32MetaDataSize;
		image->data = new unsigned char[width*height * bpp];
		image->w = width;
		image->h = height;			

		PVRTuint8* pCompressedData = &data[dataOffset];		

		PVRTDecompressPVRTC(pCompressedData,
			0,
			width,
			height,
			image->data);

		delete[] data;

		return image;		
	}

	april::Image* ImagePvr::loadMetaData(hsbase& stream)
	{		
		int size = (int)stream.size();
		uint8_t* data = new uint8_t[size];
		stream.readRaw(data, size);
		int width = 0;
		int height = 0;
		PVRHeader header = pvrGetInfo(data, size, &width, &height);
		
		if (width <= 0 || height <= 0)
		{
			hlog::error(logTag, "Could not load PVR meta data!");
			return NULL;
		}
		
		april::Image* image = new ImagePvr();
		if (header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGB)
		{
			image->format = FORMAT_RGB;			
		}
		else if (header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGBA)
		{
			image->format = FORMAT_RGBA;			
		}
		
		image->data = NULL;
		image->w = width;
		image->h = height;		

		delete[] data;

		return image;		
	}

}
#endif
