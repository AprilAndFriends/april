/// @file
/// @version 3.5
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT_WINDOW
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "WinRT_Cursor.h"

namespace april
{
	WinRT_Cursor::WinRT_Cursor() : Cursor()
	{
		this->cursor = nullptr;
	}

	WinRT_Cursor::~WinRT_Cursor()
	{
		if (this->cursor != nullptr)
		{
			this->cursor = nullptr;
		}
	}

	bool WinRT_Cursor::_create(chstr filename)
	{
		if (!Cursor::_create(filename))
		{
			return false;
		}
#ifndef _WINP8
		this->cursor = ref new CoreCursor(CoreCursorType::Custom, (unsigned int)filename);
		return true;
#else
		return false;
#endif
	}

}
#endif
