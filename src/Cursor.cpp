/// @file
/// @version 3.36
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hstring.h>

#include "Cursor.h"

namespace april
{
	Cursor::Cursor()
	{
	}
	
	Cursor::~Cursor()
	{
	}
	
	bool Cursor::_create(chstr filename)
	{
		this->filename = filename;
		return true;
	}

}
