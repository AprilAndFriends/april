/// @file
/// @author  Boris Mikic
/// @version 3.3
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
	// TODOaa - this could be optimized to immediately create a 4-byte JPG or with a substream or stream-reference class or
	// by using _loadJpg() and _loadPng() implementations that take a size parameter
	Image* Image::_loadJpt(hsbase& stream)
	{
		Image* jpg = NULL;
		Image* png = NULL;
		hstream subStream;
		int size = 0;
		unsigned char bytes[4] = {0};
		unsigned char* buffer = NULL;
		// file header ("JPT" + 1 byte for version code)
		stream.read_raw(bytes, 4);
		// read JPEG
		stream.read_raw(bytes, 4);
		size = bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
		buffer = new unsigned char[size];
		stream.read_raw(buffer, size);
		subStream.clear();
		subStream.write_raw(buffer, size);
		delete [] buffer;
		subStream.rewind();
		jpg = Image::_loadJpg(subStream);
		// read PNG
		stream.read_raw(bytes, 4);
		size = bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
		buffer = new unsigned char[size];
		stream.read_raw(buffer, size);
		subStream.clear();
		subStream.write_raw(buffer, size);
		delete [] buffer;
		subStream.rewind();
		png = Image::_loadPng(subStream);
		// combine
		Image* img = Image::create(jpg->w, jpg->h, Color::Clear, FORMAT_RGBA);
		img->write(0, 0, jpg->w, jpg->h, 0, 0, jpg);
		img->insertAlphaMap(png);
		delete jpg;
		delete png;
		return img;
	}

}
