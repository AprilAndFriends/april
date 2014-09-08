/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hmap.h>
#include <hltypes/hmutex.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "Texture.h"
#include "TextureAsync.h"

namespace april
{
	harray<Texture*> TextureAsync::textures;
	harray<hstream*> TextureAsync::streams;
	hmutex TextureAsync::queueMutex;

	hthread TextureAsync::readerThread(&TextureAsync::_read);
	bool TextureAsync::readerRunning = false;
	hmutex TextureAsync::readerMutex;

	harray<hthread*> TextureAsync::decoderThreads;

	static int cpus = 0; // needed, because certain calls are made when fetching SystemInfo that are not allowed to be made in secondary threads on some platforms

	void TextureAsync::update()
	{
		TextureAsync::queueMutex.lock();
		TextureAsync::readerMutex.lock();
		if (TextureAsync::readerRunning && !TextureAsync::readerThread.isRunning())
		{
			TextureAsync::readerThread.join();
			if (TextureAsync::textures.size() > 0) // new textures got queued in the meantime
			{
				TextureAsync::readerThread.start();
			}
			else
			{
				TextureAsync::readerRunning = false;
			}
		}
		TextureAsync::readerMutex.unlock();
		TextureAsync::queueMutex.unlock();
		// upload all ready textures to the GPU
		harray<Texture*> textures = april::rendersys->getTextures();
		foreach (Texture*, it, textures)
		{
			if ((*it)->getLoadMode() == Texture::LOAD_ASYNC && !(*it)->isLoaded() && (*it)->isLoadedAsync())
			{
				(*it)->load();
			}
		}
	}

	bool TextureAsync::queueLoad(Texture* texture)
	{
		if (cpus == 0)
		{
			cpus = april::getSystemInfo().cpuCores;
		}
		bool result = false;
		TextureAsync::queueMutex.lock();
		if (!TextureAsync::textures.contains(texture))
		{
			TextureAsync::textures += texture;
			TextureAsync::readerMutex.lock();
			if (!TextureAsync::readerRunning)
			{
				TextureAsync::readerRunning = true;
				TextureAsync::readerThread.start();
			}
			TextureAsync::readerMutex.unlock();
			result = true;
		}
		TextureAsync::queueMutex.unlock();
		return result;
	}

	bool TextureAsync::prioritizeLoad(Texture* texture)
	{
		bool result = false;
		TextureAsync::queueMutex.lock();
		if (TextureAsync::textures.contains(texture))
		{
			int index = TextureAsync::textures.index_of(texture);
			if (index >= TextureAsync::streams.size()) // if not loaded from disk yet
			{
				if (index > TextureAsync::streams.size()) // if not already at the front
				{
					TextureAsync::textures.remove_at(index);
					TextureAsync::textures.insert_at(TextureAsync::streams.size(), texture);
				}
			}
			else // if data was already loaded in RAM, but not decoded
			{
				TextureAsync::textures.remove_at(index);
				TextureAsync::textures.push_first(texture);
				TextureAsync::streams.push_first(TextureAsync::streams.remove_at(index));
			}
			result = true;
		}
		TextureAsync::queueMutex.unlock();
		return result;
	}

	void TextureAsync::_read(hthread* thread)
	{
		Texture* texture = NULL;
		hstream* stream = NULL;
		hthread* decoderThread = NULL;
		int size = 0;
		bool running = true;
		while (running)
		{
			running = false;
			// check for new queued textures
			TextureAsync::queueMutex.lock();
			if (TextureAsync::textures.size() > TextureAsync::streams.size())
			{
				texture = TextureAsync::textures[TextureAsync::streams.size()];
				TextureAsync::queueMutex.unlock();
				stream = texture->_prepareAsyncStream();
				TextureAsync::queueMutex.lock();
				int index = TextureAsync::textures.index_of(texture);
				if (stream != NULL)
				{
					// it's possible that the queue was rearranged in the meantime
					if (index >= TextureAsync::streams.size())
					{
						if (index > TextureAsync::streams.size()) // if texture was moved towards the back of the queue
						{
							// put it back to the current decoder position
							TextureAsync::textures.remove_at(index);
							TextureAsync::textures.insert_at(TextureAsync::streams.size(), texture);
						}
						TextureAsync::streams += stream;
					}
					else // the texture was moved forward in the queue
					{
						TextureAsync::streams.insert_at(index, stream);
					}
				}
				else // it was canceled
				{
					TextureAsync::textures.remove_at(index);
				}
				running = true;
			}
			size = TextureAsync::streams.size();
			TextureAsync::queueMutex.unlock();
			// create new worker threads if needed
			if (size > 0)
			{
				running = true;
				size = hmin(size, cpus) - TextureAsync::decoderThreads.size();
				for_iter (i, 0, size)
				{
					hthread* decoderThread = new hthread(&TextureAsync::_decode);
					TextureAsync::decoderThreads += decoderThread;
					decoderThread->start();
				}
			}
			// check current worker threads' status
			if (TextureAsync::decoderThreads.size() > 0)
			{
				running = true;
				for_iter (i, 0, TextureAsync::decoderThreads.size())
				{
					if (!TextureAsync::decoderThreads[i]->isRunning())
					{
						decoderThread = TextureAsync::decoderThreads.remove_at(i);
						decoderThread->join();
						delete decoderThread;
						--i;
					}
				}
			}
		}
	}

	void TextureAsync::_decode(hthread* thread)
	{
		while (true)
		{
			TextureAsync::queueMutex.lock();
			if (TextureAsync::streams.size() == 0)
			{
				TextureAsync::queueMutex.unlock();
				break;
			}
			Texture* texture = TextureAsync::textures.remove_first();
			hstream* stream = TextureAsync::streams.remove_first();
			TextureAsync::queueMutex.unlock();
			texture->_loadFromAsyncStream(stream);
			delete stream;
		}
	}

}
