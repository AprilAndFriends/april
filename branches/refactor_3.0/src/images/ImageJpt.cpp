/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hsbase.h>
#include <hltypes/hstream.h>

#include "Image.h"

namespace april
{
	Image* Image::_loadJpt(hsbase& stream)
	{
		Image* jpg = NULL;
		Image* png = NULL;
		hstream subStream;
		int size;
		unsigned char bytes[4];
		unsigned char* buffer;
		// file header ("JPT" + 1 byte for version code)
		stream.read_raw(bytes, 4);
		// read JPEG
		stream.read_raw(bytes, 4);
		size = bytes[0] + bytes[1] * 256 + bytes[2] * 256 * 256 + bytes[3] * 256 * 256 * 256;
		buffer = new unsigned char[size];
		stream.read_raw(buffer, size);
		subStream.clear();
		subStream.write_raw(buffer, size);
		delete [] buffer;
		subStream.rewind();
		jpg = Image::_loadJpg(subStream);
		// read PNG
		stream.read_raw(bytes, 4);
		size = bytes[0] + bytes[1] * 256 + bytes[2] * 256 * 256 + bytes[3] * 256 * 256 * 256;
		buffer = new unsigned char[size];
		stream.read_raw(buffer, size);
		subStream.clear();
		subStream.write_raw(buffer, size);
		delete [] buffer;
		subStream.rewind();
		png = Image::_loadPng(subStream);
		// combine
		Image* img = Image::create(jpg->w, jpg->h);
		img->copyImage(jpg);
		img->insertAsAlphaMap(png);
		delete jpg;
		delete png;
		return img;
	}

}
