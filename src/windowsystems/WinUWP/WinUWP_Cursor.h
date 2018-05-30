/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
///
/// Defines a WinUWP cursor.

#ifdef _WINUWP_WINDOW
#ifndef APRIL_WINUWP_CURSOR_H
#define APRIL_WINUWP_CURSOR_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Cursor.h"

using namespace Windows::UI::Core;

namespace april
{
	class WinUWP_Cursor : public Cursor
	{
	public:
		WinUWP_Cursor(bool fromResource);
		~WinUWP_Cursor();

		HL_DEFINE_GET(CoreCursor^, cursor, Cursor);

	protected:
		CoreCursor^ cursor;

		bool _create(chstr filename);

	};
	
}

#endif
#endif
