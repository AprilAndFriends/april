/// @file
/// @version 3.5
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
	SDL_Cursor::SDL_Cursor() : Cursor(), cursor(NULL)
	{
	}

	SDL_Cursor::~SDL_Cursor()
	{
		if (this->cursor != NULL)
		{
			SDL_FreeCursor(this->cursor);
			this->cursor = NULL;
		}
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
		Image* image = Image::createFromResource(filename, Image::FORMAT_RGBA);
		SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image->data, image->w, image->h, image->getBpp() * 8, image->w * image->getBpp(), 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
		this->cursor = SDL_CreateColorCursor(surface, 0, 0);
		SDL_FreeSurface(surface);
		delete image;
		return true;
	}

}
#endif
