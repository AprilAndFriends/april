/// @file
/// @version 5.0
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

#define PVR_HEADER_SIZE 52

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
		Image* image = new Image();
		image->w = header.width;
		image->h = header.height;
		image->internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		image->compressedSize = header.size - PVR_HEADER_SIZE;
		image->format = Image::Format::Compressed;
		image->data = new unsigned char[image->compressedSize];
		memcpy(image->data, &pvrData[PVR_HEADER_SIZE], image->compressedSize);
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
		image->compressedSize = header.size - PVR_HEADER_SIZE;
		image->format = Image::Format::Compressed;
		image->data = NULL;
		return image;
	}

	Image* Image::_readMetaDataPvrz(hsbase& stream)
	{
		return Image::_readMetaDataPvrz(stream, (int)stream.size());
	}

}
#endif
