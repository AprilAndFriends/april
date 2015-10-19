/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "aprilpix.h"

namespace aprilpix
{
	hstr logTag = "aprilpix";

	void init()
	{
		hlog::write(logTag, "Initializing AprilPIX");
	}

	void destroy()
	{
		hlog::write(logTag, "Destroying AprilPix");
	}

}
