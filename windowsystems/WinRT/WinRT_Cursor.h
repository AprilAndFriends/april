/// @file
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
/// Defines a WinRT cursor.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_CURSOR_H
#define APRIL_WINRT_CURSOR_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Cursor.h"

#ifndef _WINP8
using namespace Windows::UI::Core;
#endif

namespace april
{
	class WinRT_Cursor : public Cursor
	{
	public:
		WinRT_Cursor();
		~WinRT_Cursor();

#ifndef _WINP8
		HL_DEFINE_GET(CoreCursor^, cursor, Cursor);

	protected:
		CoreCursor^ cursor;
#endif

		bool _create(chstr filename);

	};
	
}

#endif
#endif
