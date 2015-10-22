/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _PVR
#include "aprilpix.h"
#include "ImagePvr.h"

#include <hltypes/hlog.h>
#include <algorithm>
#include <iostream>

namespace aprilpix
{

	ImagePvr::ImagePvr() : april::Image()
	{
	}

	ImagePvr::~ImagePvr()
	{
	}

	PVRTextureHeaderV3 ImagePvr::pvrGetInfo(uint8_t* data, int size, int* width, int* height)
	{
		PVRTextureHeaderV3 header;
		memcpy(&header, data, PVR_HEADER_SIZE);
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
		PVRTextureHeaderV3 header = pvrGetInfo(data, size, &width, &height);
		delete[] data;
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
			bpp = 3;
		}
		else if (header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGBA)		
		{
			image->format = FORMAT_RGBA;
			bpp = 4;
		}
		
		int dataOffset = PVR_HEADER_SIZE + header.u32MetaDataSize;
		image->data = new unsigned char[width*height * bpp];
		image->w = width;
		image->h = height;

		int numMipMaps = header.u32MIPMapCount;
		int numFaces = header.u32NumFaces;
		int numSurfaces = header.u32NumSurfaces;

		int currentPixel = 0;
		for (int i = 0; i < numMipMaps; i++)
			for (int j = 0; j < numSurfaces; j++)
				for (int k = 0; k < numFaces; k++)
					for (int d = 0; d < header.u32Depth; d++)
						for (int row = 0; row < height; row++)
							for (int x = 0; x < width; x++)
							{								
								image->data[row * height + x] = (unsigned char)&data[dataOffset + currentPixel];
								currentPixel++;
							}

		return image;
	}

	april::Image* ImagePvr::loadMetaData(hsbase& stream)
	{
		int size = (int)stream.size();
		uint8_t* data = new uint8_t[size];
		stream.readRaw(data, size);
		int width = 0;
		int height = 0;
		PVRTextureHeaderV3 header = pvrGetInfo(data, size, &width, &height);
		delete[] data;
		if (width <= 0 || height <= 0)
		{
			hlog::error(logTag, "Could not load PVR meta data!");
			return NULL;
		}
		int bpp = 1;
		april::Image* image = new ImagePvr();
		if (header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGB)
		{
			image->format = FORMAT_RGB;
			bpp = 3;
		}
		else if (header.u64PixelFormat == ePVRTPF_PVRTCI_4bpp_RGBA)
		{
			image->format = FORMAT_RGBA;
			bpp = 4;
		}
		int dataOffset = PVR_HEADER_SIZE + header.u32MetaDataSize;
		image->data = NULL;
		image->w = width;
		image->h = height;

		return image;
	}
}
#endif
