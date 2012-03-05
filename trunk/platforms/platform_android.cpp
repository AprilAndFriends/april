/// @file
/// @author  Boris Mikic
/// @version 1.51
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <gtypes/Vector2.h>

#include "Platform.h"

namespace april
{
	extern gvec2 androidResolution; // TODO

	gvec2 getDisplayResolution()
	{
		return april::androidResolution; // TODO
	}

}
#endif
