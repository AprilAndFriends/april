/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstring.h>
#include <zlib.h>

#include "april.h"
#include "zlibUtil.h"

namespace april
{
	static hmutex zlibMutex;

	unsigned char* zlibDecompress(int streamSize, int compressedSize, hsbase& stream)
	{
		// zlib inflate init
		z_stream zlibStream;
		memset(&zlibStream, 0, sizeof(zlibStream));
		hmutex::ScopeLock lock(&zlibMutex);
		int result = inflateInit(&zlibStream);
		if (result != Z_OK)
		{
			hlog::error(logTag, "zlib Error: " + hstr(result));
			return NULL;
		}
		unsigned char* data = new unsigned char[streamSize];
		unsigned char* input = new unsigned char[compressedSize];
		stream.readRaw(input, compressedSize);
		// decompress
		zlibStream.next_in = input;
		zlibStream.avail_in = compressedSize;
		zlibStream.next_out = data;
		zlibStream.avail_out = streamSize;
		if (inflate(&zlibStream, Z_FINISH) == Z_STREAM_ERROR)
		{
			delete[] data;
			data = NULL;
		}
		inflateEnd(&zlibStream);
		delete[] input;
		return data;
	}

}
