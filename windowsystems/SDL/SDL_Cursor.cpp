/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.36
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _SDL_WINDOW
#include <SDL/SDL.h>

#include <hltypes/hstring.h>

#include "SDL_Cursor.h"
#include "Image.h"

namespace april
{
	SDL_Cursor::SDL_Cursor() : Cursor()
	{
	}

	SDL_Cursor::~SDL_Cursor()
	{
		// TODO
	}

	bool SDL_Cursor::_create(chstr filename)
	{
		if (!Cursor::_create(filename))
		{
			return false;
		}
		if (filename == "")
		{
			return false;
		}
		// TODO
		return false;
	}

}
#endif
