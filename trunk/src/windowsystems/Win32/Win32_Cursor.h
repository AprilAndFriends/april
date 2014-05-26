/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.36
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
///
/// Defines a Win32 cursor.

#ifdef _WIN32_WINDOW
#ifndef APRIL_WIN32_CURSOR_H
#define APRIL_WIN32_CURSOR_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Cursor.h"

namespace april
{
	class Win32_Cursor : public Cursor
	{
	public:
		Win32_Cursor();
		~Win32_Cursor();

		HL_DEFINE_GET(HCURSOR, cursor, Cursor);

	protected:
		HCURSOR cursor;

		bool _create(chstr filename);

	};
	
}

#endif
#endif
