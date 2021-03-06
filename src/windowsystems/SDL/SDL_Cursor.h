/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
///
/// Defines an SDL cursor.

#ifdef _SDL_WINDOW
#ifndef APRIL_SDL_CURSOR_H
#define APRIL_SDL_CURSOR_H

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "Cursor.h"

namespace april
{
	class SDL_Cursor : public Cursor
	{
	public:
		SDL_Cursor(bool fromResource);
		~SDL_Cursor();

		HL_DEFINE_GET(::SDL_Cursor*, cursor, Cursor);

	protected:
		::SDL_Cursor* cursor;

		bool _create(chstr filename);

	};
	
}

#endif
#endif
