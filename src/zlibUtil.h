/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines utility functions for zlib usage.

#ifndef APRIL_ZLIB_UTIL_H
#define APRIL_ZLIB_UTIL_H

#include <hltypes/hsbase.h>

namespace april
{
	unsigned char* zlibDecompress(int streamSize, int compressedSize, hsbase& stream);

}
#endif
