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

	hthread TextureAsync::readerThread(&TextureAsync::_read, "APRIL async loader");
	bool TextureAsync::readerRunning = false;

	harray<hthread*> TextureAsync::decoderThreads;

	static int cpus = 0; // needed, because certain calls are made when fetching SystemInfo that are not allowed to be made in secondary threads on some platforms

	void TextureAsync::update()
	{
		hmutex::ScopeLock lock(&TextureAsync::queueMutex);
		if (TextureAsync::readerRunning && !TextureAsync::readerThread.isRunning())
		{
			TextureAsync::readerThread.join();
			TextureAsync::readerRunning = false;
		}
		if (!TextureAsync::readerRunning && TextureAsync::textures.size() > 0) // new textures got queued in the meantime
		{
			TextureAsync::readerRunning = true;
			TextureAsync::readerThread.start();
		}
		lock.release();
		// upload all ready textures to the GPU
		int maxCount = april::getMaxAsyncTextureUploadsPerFrame();
		int count = 0;
		harray<Texture*> textures = april::rendersys->getTextures();
		foreach (Texture*, it, textures)
		{
			// only async on-demand textures shouldn't be loaded
			if ((*it)->getLoadMode() != Texture::LOAD_ASYNC_ON_DEMAND && (*it)->isLoadedAsync())
			{
				(*it)->load();
				++count;
				if (maxCount > 0 && count >= maxCount)
				{
					break; // only 'maxCount' textures per frame!
				}
			}
		}
	}

	bool TextureAsync::queueLoad(Texture* texture)
	{
		if (cpus == 0)
		{
			cpus = april::getSystemInfo().cpuCores;
		}
		hmutex::ScopeLock lock(&TextureAsync::queueMutex);
		if (TextureAsync::textures.has(texture))
		{
			return false;
		}
		TextureAsync::textures += texture;
		if (!TextureAsync::readerRunning)
		{
			TextureAsync::readerRunning = true;
			TextureAsync::readerThread.start();
		}
		return true;
	}

	bool TextureAsync::prioritizeLoad(Texture* texture)
	{
		hmutex::ScopeLock lock(&TextureAsync::queueMutex);
		if (!TextureAsync::textures.has(texture))
		{
			return false;
		}
		int index = TextureAsync::textures.indexOf(texture);
		if (index >= TextureAsync::streams.size()) // if not loaded from disk yet
		{
			if (index > TextureAsync::streams.size()) // if not already at the front
			{
				TextureAsync::textures.removeAt(index);
				TextureAsync::textures.insertAt(TextureAsync::streams.size(), texture);
			}
		}
		else if (index > 0) // if data was already loaded in RAM, but not decoded and not already at the front
		{
			TextureAsync::textures.removeAt(index);
			TextureAsync::textures.addFirst(texture);
			TextureAsync::streams.addFirst(TextureAsync::streams.removeAt(index));
		}
		return true;
	}

	bool TextureAsync::isRunning()
	{
		hmutex::ScopeLock lock(&TextureAsync::queueMutex);
		return TextureAsync::readerRunning;
	}

	void TextureAsync::_read(hthread* thread)
	{
		Texture* texture = NULL;
		hstream* stream = NULL;
		hthread* decoderThread = NULL;
		int index = 0;
		int size = 0;
		bool running = true;
		hmutex::ScopeLock lock;
		int maxWaitingCount = 0;
		while (running)
		{
			running = false;
			maxWaitingCount = getMaxWaitingAsyncTextures(); // keep this value up to date in every iteration
			// check for new queued textures
			lock.acquire(&TextureAsync::queueMutex);
			if (TextureAsync::textures.size() > TextureAsync::streams.size())
			{
				running = true;
				if (maxWaitingCount <= 0 || TextureAsync::streams.size() < maxWaitingCount)
				{
					texture = TextureAsync::textures[TextureAsync::streams.size()];
					lock.release();
					stream = texture->_prepareAsyncStream();
					lock.acquire(&TextureAsync::queueMutex);
					index = TextureAsync::textures.indexOf(texture); // it's possible that the queue was rearranged in the meantime
					if (stream != NULL)
					{
						if (index >= TextureAsync::streams.size())
						{
							if (index > TextureAsync::streams.size()) // if texture was moved towards the back of the queue
							{
								// put it back to the current decoder position
								TextureAsync::textures.removeAt(index);
								TextureAsync::textures.insertAt(TextureAsync::streams.size(), texture);
							}
							TextureAsync::streams += stream;
						}
						else // the texture was moved forward in the queue
						{
							TextureAsync::streams.insertAt(index, stream);
						}
					}
					else // it was canceled
					{
						TextureAsync::textures.removeAt(index);
					}
				}
				else
				{
					lock.release();
					hthread::sleep(1.0f);
					lock.acquire(&TextureAsync::queueMutex);
				}
			}
			size = TextureAsync::streams.size();
			lock.release();
			// create new worker threads if needed
			if (size > 0)
			{
				running = true;
				size = hmin(size, cpus) - TextureAsync::decoderThreads.size();
				for_iter (i, 0, size)
				{
					decoderThread = new hthread(&TextureAsync::_decode, "APRIL async decoder");
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
						decoderThread = TextureAsync::decoderThreads.removeAt(i);
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
		Texture* texture = NULL;
		hstream* stream = NULL;
		hmutex::ScopeLock lock(&TextureAsync::queueMutex);
		while (TextureAsync::streams.size() > 0)
		{
			texture = TextureAsync::textures.removeFirst();
			stream = TextureAsync::streams.removeFirst();
			lock.release();
			texture->_decodeFromAsyncStream(stream);
			delete stream;
			lock.acquire(&TextureAsync::queueMutex);
		}
	}

}
