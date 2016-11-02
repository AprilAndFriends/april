/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _ANDROID
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

#define ETCX_HEADER_HAS_ALPHA_BIT 0x1
#define ETCX_HEADER_IS_ZLIB_COMPRESSED_BIT 0x2

namespace april
{
	struct EtcxHeader
	{
		char signature[4];
		unsigned int flags;
		unsigned int width;
		unsigned int height;
		unsigned int size;
		unsigned int compressedSize;
	};

	static hmutex mutex;

	Image* Image::_loadEtcx(hsbase& stream, int size)
	{
		EtcxHeader header;
		stream.readRaw(&header, sizeof(EtcxHeader));
		if (hstr(header.signature, sizeof(header.signature)) != "ETCX")
		{
			return NULL;
		}
		Image* image = new Image();
		image->w = header.width;
		image->h = header.height;
		image->internalFormat = GL_ETC1_RGB8_OES;
		image->compressedSize = header.size;
		if ((header.flags & ETCX_HEADER_HAS_ALPHA_BIT) != 0)
		{
			image->internalFormat = GL_ETCX_RGBA8_OES_HACK;
		}
		image->format = Image::FORMAT_COMPRESSED;
		if ((header.flags & ETCX_HEADER_IS_ZLIB_COMPRESSED_BIT) == 0)
		{
			image->data = new unsigned char[image->compressedSize];
			stream.readRaw(image->data, image->compressedSize);
			return image;
		}
		// zlib inflate init
		z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;
		zlibStream.avail_in = 0;
		zlibStream.next_in = Z_NULL;
		zlibStream.avail_out = 0;
		zlibStream.next_out = Z_NULL;
		hmutex::ScopeLock lock(&mutex);
		int result = inflateInit(&zlibStream);
		if (result != Z_OK)
		{
			hlog::error(logTag, "zlib Error: " + hstr(result));
			delete image;
			return NULL;
		}
		image->data = new unsigned char[image->compressedSize];
		unsigned char* input = new unsigned char[header.compressedSize];
		stream.readRaw(input, header.compressedSize);
		// decompress
		zlibStream.next_in = input;
		zlibStream.avail_in = header.compressedSize;
		zlibStream.next_out = image->data;
		zlibStream.avail_out = image->compressedSize;
		if (inflate(&zlibStream, Z_FINISH) == Z_STREAM_ERROR)
		{
			delete image;
			image = NULL;
		}
		inflateEnd(&zlibStream);
		delete[] input;
		return image;
	}

	Image* Image::_loadEtcx(hsbase& stream)
	{
		return Image::_loadEtcx(stream, (int)stream.size());
	}

	Image* Image::_readMetaDataEtcx(hsbase& stream, int size)
	{
		EtcxHeader header;
		stream.readRaw(&header, sizeof(EtcxHeader));
		if (hstr(header.signature, sizeof(header.signature)) != "ETCX")
		{
			return NULL;
		}
		Image* image = new Image();
		image->w = header.width;
		image->h = header.height;
		image->internalFormat = GL_ETC1_RGB8_OES;
		image->compressedSize = header.size;
		if ((header.flags & ETCX_HEADER_HAS_ALPHA_BIT) != 0)
		{
			image->internalFormat = GL_ETCX_RGBA8_OES_HACK;
		}
		image->format = Image::FORMAT_COMPRESSED;
		image->data = NULL;
		return image;
	}

	Image* Image::_readMetaDataEtcx(hsbase& stream)
	{
		return Image::_readMetaDataEtcx(stream, (int)stream.size());
	}

}
#endif
