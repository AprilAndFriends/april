/// @file
/// @version 5.2
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WIN32_WINDOW
#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hstring.h>
#include <hltypes/hrdir.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>

#include "Win32_Cursor.h"

namespace april
{
	Win32_Cursor::Win32_Cursor(bool fromResource) :
		Cursor(fromResource),
		cursor(NULL)
	{
	}
	
	Win32_Cursor::~Win32_Cursor()
	{
		if (this->cursor != NULL)
		{
			DestroyCursor(this->cursor);
			this->cursor = NULL;
		}
	}
	
	bool Win32_Cursor::_create(chstr filename)
	{
		if (filename == "")
		{
			return false;
		}
		hstr path = filename;
		hstr archivePath = hresource::getMountedArchives().tryGet("", "");
		if (this->fromResource && archivePath != "")
		{
			path = hrdir::joinPath(archivePath, filename);
		}
		if (!Cursor::_create(path))
		{
			return false;
		}
		// does not actually differ between hresource and hfile
		this->cursor = LoadCursorFromFileW(path.wStr().c_str());
		return true;
	}
	
}
#endif
