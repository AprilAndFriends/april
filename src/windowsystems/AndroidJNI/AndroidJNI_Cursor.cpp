/// @file
/// @version 4.5
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _ANDROIDJNI_WINDOW
#include "AndroidJNI_Cursor.h"

namespace april
{
	AndroidJNI_Cursor::AndroidJNI_Cursor(bool fromResource) : Cursor(fromResource)
	{
	}

	AndroidJNI_Cursor::~AndroidJNI_Cursor()
	{
	}

}
#endif
