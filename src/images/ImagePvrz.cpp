/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _IMAGE_PVR
#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>

#include <stdio.h>

#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmutex.h>
#include <hltypes/hresource.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>
#include <zlib.h>

#include "april.h"
#include "OpenGL_RenderSystem.h"
#include "Image.h"
#include "Platform.h"
#include "zlibUtil.h"

#define READ_LITTLE_ENDIAN_UINT32(location, offset) ((location)[offset] | ((location)[offset + 1] << 8) | ((location)[offset + 2] << 16) | ((location)[offset + 3] << 24))

#ifndef PVR_Texture_Header
struct PVR_Texture_Header
{
	uint32_t dwHeaderSize;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwMipMapCount;
	uint32_t dwpfFlags;
	uint32_t dwTextureDataSize;
	uint32_t dwBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwAlphaBitMask;
	uint32_t dwPVR;
	uint32_t dwNumSurfs;
};
#endif

namespace april
{
	struct PvrzHeader
	{
		char signature[4];
		unsigned int flags;
		unsigned int width;
		unsigned int height;
		unsigned int size;
		unsigned int compressedSize;
	};

	static hmutex mutex;

	Image* Image::_loadPvrz(hsbase& stream, int size)
	{
		PvrzHeader header;
		stream.readRaw(&header, sizeof(PvrzHeader));
		if (hstr(header.signature, sizeof(header.signature)) != "PVRZ")
		{
			return NULL;
		}
		unsigned char* pvrData = zlibDecompress(header.size, header.compressedSize, stream);
		if (pvrData == NULL)
		{
			return NULL;
		}
		struct PVR_Texture_Header pvrHeader;
		pvrHeader.dwHeaderSize = READ_LITTLE_ENDIAN_UINT32(pvrData, 0);
		pvrHeader.dwHeight = READ_LITTLE_ENDIAN_UINT32(pvrData, 4);
		pvrHeader.dwWidth = READ_LITTLE_ENDIAN_UINT32(pvrData, 8);
		pvrHeader.dwMipMapCount = READ_LITTLE_ENDIAN_UINT32(pvrData, 12);
		pvrHeader.dwpfFlags = READ_LITTLE_ENDIAN_UINT32(pvrData, 16);
		pvrHeader.dwTextureDataSize = READ_LITTLE_ENDIAN_UINT32(pvrData, 20);
		pvrHeader.dwBitCount = READ_LITTLE_ENDIAN_UINT32(pvrData, 24);
		pvrHeader.dwRBitMask = READ_LITTLE_ENDIAN_UINT32(pvrData, 28);
		pvrHeader.dwGBitMask = READ_LITTLE_ENDIAN_UINT32(pvrData, 32);
		pvrHeader.dwBBitMask = READ_LITTLE_ENDIAN_UINT32(pvrData, 36);
		pvrHeader.dwAlphaBitMask = READ_LITTLE_ENDIAN_UINT32(pvrData, 40);
		pvrHeader.dwPVR = READ_LITTLE_ENDIAN_UINT32(pvrData, 44);
		pvrHeader.dwNumSurfs = READ_LITTLE_ENDIAN_UINT32(pvrData, 48);
		Image* image = new Image();
		image->w = header.width;
		image->h = header.height;
		image->internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		image->compressedSize = header.size;
		image->format = Image::FORMAT_COMPRESSED;
		image->data = new unsigned char[image->compressedSize];
		memcpy(image->data, pvrData + sizeof(struct PVR_Texture_Header), image->compressedSize);
		delete[] pvrData;
		return image;
	}

	Image* Image::_loadPvrz(hsbase& stream)
	{
		return Image::_loadPvrz(stream, (int)stream.size());
	}

	Image* Image::_readMetaDataPvrz(hsbase& stream, int size)
	{
		PvrzHeader header;
		stream.readRaw(&header, sizeof(PvrzHeader));
		if (hstr(header.signature, sizeof(header.signature)) != "PVRZ")
		{
			return NULL;
		}
		Image* image = new Image();
		image->w = header.width;
		image->h = header.height;
		image->internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		image->compressedSize = header.size;
		image->format = Image::FORMAT_COMPRESSED;
		image->data = NULL;
		return image;
	}

	Image* Image::_readMetaDataPvrz(hsbase& stream)
	{
		return Image::_readMetaDataPvrz(stream, (int)stream.size());
	}

}
#endif
