/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "Application.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "TextureAsync.h"
#include "UpdateDelegate.h"
#include "Window.h"

namespace april
{
	Application* application = NULL;
	
	Application::Application(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)()) : running(true), started(false), autoPresentFrame(false),
		timeDelta(0.0f), fps(0), fpsCount(0), fpsTimer(0.0f), fpsResolution(0.5f), timeDeltaMaxLimit(0.1f), updateThread(&_asyncUpdate)
	{
		this->aprilApplicationInit = aprilApplicationInit;
		this->aprilApplicationDestroy = aprilApplicationDestroy;
#ifdef _ANDROID
		this->autoPresentFrame = true;
#endif
	}
	
	Application::~Application()
	{
	}

	void Application::init()
	{
		this->updateThread.start();
		while (!this->started)
		{
			hthread::sleep(0.1f);
			if (april::rendersys != NULL)
			{
				TextureAsync::update();
				april::rendersys->update(0.0f); // might require some rendering
			}
		}
	}

	void Application::destroy()
	{
		this->running = false;
		this->updateThread.join();
	}

	void Application::enterMainLoop()
	{
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
		this->running = true;
		while (this->running)
		{
			this->update();
		}
	}

	void Application::update()
	{
		TextureAsync::update();
		float timeDelta = this->timer.diff(true);
		if (!april::window->isFocused())
		{
			timeDelta = 0.0f;
		}
		this->fpsTimer += timeDelta;
		if (this->timeDeltaMaxLimit > 0.0f)
		{
			timeDelta = hmin(timeDelta, this->timeDeltaMaxLimit);
		}
		if (this->fpsTimer > 0.0f)
		{
			++this->fpsCount;
			if (this->fpsTimer >= this->fpsResolution)
			{
				this->fps = hceil(this->fpsCount / this->fpsTimer);
				this->fpsCount = 0;
				this->fpsTimer = 0.0f;
			}
		}
		else
		{
			this->fps = 0;
			this->fpsCount = 0;
		}
		hmutex::ScopeLock lock(&this->updateMutex);
		this->timeDelta += timeDelta;
		lock.release();
		april::window->checkEvents();
		april::rendersys->update(timeDelta);
		// this consumes the timeDelta after the frame is done
		if (!april::window->isFocused())
		{
			this->timer.diff(true);
		}
	}

	void Application::finish()
	{
		this->running = false;
	}

	void Application::_asyncUpdate(hthread* thread)
	{
		(*april::application->aprilApplicationInit)();
		april::application->started = true;
		float timeDelta = 0.0f;
		UpdateDelegate* updateDelegate = NULL;
		hmutex::ScopeLock lock;
		while (april::application->running)
		{
			lock.acquire(&april::application->updateMutex);
			timeDelta = april::application->timeDelta;
			april::application->timeDelta = 0.0f;
			lock.release();
			if (!april::window->update(timeDelta))
			{
				april::application->running = false;
			}
			updateDelegate = april::window->getUpdateDelegate();
			if (updateDelegate != NULL)
			{
#ifndef _ANDROID
				updateDelegate->onPresentFrame();
#else // on AndroidJNI the backend does buffer swapping so this single call needs to be ignored
				april::window->setPresentFrameEnabled(false);
				updateDelegate->onPresentFrame();
				april::window->setPresentFrameEnabled(true);
#endif
			}
			while (april::rendersys->getAsyncQueuesCount() > april::rendersys->getFrameAdvanceUpdates())
			{
				hthread::sleep(0.01f);
			}
		}
		(*april::application->aprilApplicationDestroy)();
	}

}
