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
/// Defines a Win32 cursor.

#ifndef APRIL_MAC_CURSOR_H
#define APRIL_MAC_CURSOR_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Cursor.h"

namespace april
{
	class Mac_Cursor : public Cursor
	{
	public:
		Mac_Cursor(bool fromResource);
		~Mac_Cursor();
		
		inline NSCursor* getNSCursor() { return systemCursor; }
		
	protected:
		NSCursor* systemCursor;
		
		bool _create(chstr filename);
		
	};
}

#endif
