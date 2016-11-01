/// @file
/// @version 4.1
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
#include <hltypes/hresource.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "OpenGL_RenderSystem.h"
#include "Image.h"

#define ETCX_HEADER_HAS_ALPHA_BIT 1

namespace april
{
	struct EtcxHeader
	{
		char signature[4];
		unsigned int flags;
		unsigned int width;
		unsigned int height;
	};

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
		image->compressedSize = (image->w / 4) * (image->h / 4) * 8;
		if ((header.flags & ETCX_HEADER_HAS_ALPHA_BIT) != 0)
		{
			image->internalFormat = GL_ETCX_RGBA8_OES_HACK;
			image->compressedSize *= 2;
		}
		image->format = Image::FORMAT_COMPRESSED;
		image->data = new unsigned char[image->compressedSize];
		stream.readRaw(image->data, image->compressedSize);
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
		image->compressedSize = (image->w / 4) * (image->h / 4) * 8;
		if ((header.flags & ETCX_HEADER_HAS_ALPHA_BIT) != 0)
		{
			image->internalFormat = GL_ETCX_RGBA8_OES_HACK;
			image->compressedSize *= 2;
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
