/// @file
/// @version 5.2
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UWP_WINDOW
#include <hltypes/hfbase.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "UWP_Cursor.h"
#include "UWP_Window.h"

namespace april
{
	UWP_Cursor::UWP_Cursor(bool fromResource) :
		Cursor(fromResource)
	{
		this->cursor = nullptr;
	}

	UWP_Cursor::~UWP_Cursor()
	{
		if (this->cursor != nullptr)
		{
			this->cursor = nullptr;
		}
	}

	bool UWP_Cursor::_create(chstr filename)
	{
#ifndef _WINP8
		hmap<hstr, int> cursorMappings;
		harray<hstr> lines = april::window->getParam(UWP_CURSOR_MAPPINGS).split('\n', -1, true);
		harray<hstr> data;
		foreach (hstr, it, lines)
		{
			data = (*it).split(' ', 1);
			if (data.size() == 2)
			{
				cursorMappings[data[1]] = (int)data[0];
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
