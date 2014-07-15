/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
///
/// Defines an Android JNI cursor.

#ifdef _ANDROIDJNI_WINDOW
#ifndef APRIL_ANDROIDJNI_CURSOR_H
#define APRIL_ANDROIDJNI_CURSOR_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Cursor.h"

namespace april
{
	class AndroidJNI_Cursor : public Cursor
	{
	public:
		AndroidJNI_Cursor();
		~AndroidJNI_Cursor();

	};
	
}

#endif
#endif
