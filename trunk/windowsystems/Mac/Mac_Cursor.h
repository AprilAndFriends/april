/// @file
/// @author  Kresimir Spes
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

#ifndef APRIL_MAC_CURSOR_H
#define APRIL_MAC_CURSOR_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "Cursor.h"

namespace april
{
	class Mac_Cursor : public Cursor
	{
		NSCursor* mCursor;
	public:
		Mac_Cursor();
		~Mac_Cursor();
		NSCursor* getNSCursor() { return mCursor; }
	protected:
		bool _create(chstr filename);
	};
}

#endif
