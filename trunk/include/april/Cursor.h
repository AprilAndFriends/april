/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a Cursor object.

#ifndef APRIL_CURSOR_H
#define APRIL_CURSOR_H

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class Window;

	class aprilExport Cursor
	{
	public:
		friend class Window;

		virtual ~Cursor();

		HL_DEFINE_GET(hstr, filename, Filename);
		
	protected:
		hstr filename;
		
		Cursor();

		virtual bool _create(chstr filename);

	};

}

#endif
