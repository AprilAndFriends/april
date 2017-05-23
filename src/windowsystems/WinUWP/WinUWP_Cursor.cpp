/// @file
/// @version 4.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINUWP_WINDOW
#include <hltypes/hfbase.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "WinUWP_Cursor.h"
#include "WinUWP_Window.h"

namespace april
{
	WinUWP_Cursor::WinUWP_Cursor(bool fromResource) : Cursor(fromResource)
	{
		this->cursor = nullptr;
	}

	WinUWP_Cursor::~WinUWP_Cursor()
	{
		if (this->cursor != nullptr)
		{
			this->cursor = nullptr;
		}
	}

	bool WinUWP_Cursor::_create(chstr filename)
	{
#ifndef _WINP8
		hmap<hstr, int> cursorMappings;
		harray<hstr> lines = april::window->getParam(WINUWP_CURSOR_MAPPINGS).split('\n', -1, true);
		harray<hstr> data;
		foreach (hstr, it, lines)
		{
			data = (*it).split(' ', 1);
			if (data.size() == 2)
			{
				cursorMappings[data[1]] = (unsigned int)data[0];
			}
		}
		int id = (cursorMappings.hasKey(filename) ? cursorMappings[filename] : cursorMappings.tryGet(hfbase::withoutExtension(filename), -1));
		if (id < 0)
		{
			return false;
		}
#endif
		if (!Cursor::_create(filename))
		{
			return false;
		}
		// does not actually differ between hresource and hfile as only internal .exe resources can be cursors
#ifndef _WINP8
		this->cursor = ref new CoreCursor(CoreCursorType::Custom, id);
		return (this->cursor != nullptr);
#else
		return false;
#endif
	}

}
#endif
