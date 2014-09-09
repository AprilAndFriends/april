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
/// Defines utilities for asynchronous texture loading.

#ifndef APRIL_TEXTURE_ASYNC_H
#define APRIL_TEXTURE_ASYNC_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hlist.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hthread.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class Texture;

	class TextureAsync
	{
	public:
		static void update();
		static bool queueLoad(Texture* texture);
		static bool prioritizeLoad(Texture* texture);
		static bool isRunning();

	protected:
		static harray<Texture*> textures;
		static harray<hstream*> streams;
		static hmutex queueMutex;

		static hthread readerThread;
		static bool readerRunning;
		static hmutex readerMutex;

		static harray<hthread*> decoderThreads;

		static void _read(hthread* thread);
		static void _decode(hthread* thread);

	private: // prevents inheritance and instantiation
		TextureAsync() { }
		~TextureAsync() { }

	};
	
}

#endif
