/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Application.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "TextureAsync.h"
#include "UpdateDelegate.h"
#include "Window.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(Application::State,
	(
		HL_ENUM_DEFINE(Application::State, Idle);
		HL_ENUM_DEFINE(Application::State, Starting);
		HL_ENUM_DEFINE(Application::State, Running);
		HL_ENUM_DEFINE(Application::State, Stopping);
		HL_ENUM_DEFINE(Application::State, Stopped);
	));

	Application* application = NULL;
	
	Application::Application(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)()) : state(State::Idle), autoPresentFrame(false),
		timeDelta(0.0f), fps(0), fpsCount(0), fpsTimer(0.0f), fpsResolution(0.5f), timeDeltaMaxLimit(0.1f), updateThread(&_asyncUpdate, "APRIL Async Update")
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
		this->state = State::Starting;
		this->updateThread.start();
		while (this->state == State::Starting)
		{
			if (april::window != NULL && april::rendersys != NULL)
			{
				this->_updateSystem();
			}
			hthread::sleep(0.001f);
		}
	}

	void Application::destroy()
	{
		this->state = State::Idle;
		this->updateThread.join();
	}

	void Application::enterMainLoop()
	{
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
		while (this->state == State::Running)
		{
			this->update();
		}
		// processing remaining commands from other thread
		while (this->state == State::Stopping)
		{
			this->_updateSystem();
		}
		// finish everything up
		this->_updateSystem();
		// done
		this->state = State::Idle;
	}

	void Application::update()
	{
		TextureAsync::update();
		april::window->checkEvents();
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
		hmutex::ScopeLock lock(&this->updateMutex);
		this->timeDelta += timeDelta;
		lock.release();
		april::rendersys->update(timeDelta);
		// this consumes the timeDelta after the frame is done
		if (!april::window->isFocused())
		{
			this->timer.diff(true);
		}
	}

	void Application::_updateSystem()
	{
		TextureAsync::update();
		april::window->checkEvents();
		april::rendersys->update(0.0f); // might require some rendering
	}

	void Application::_updateFps()
	{
		if (this->fpsTimer > 0.0f)
		{
			++this->fpsCount;
			if (this->fpsTimer >= this->fpsResolution)
			{
				this->fps = hceil(this->fpsCount / this->fpsTimer);
				this->fpsCount = 0;
				this->fpsTimer -= this->fpsResolution;
			}
		}
		else
		{
			this->fps = 0;
			this->fpsCount = 0;
		}
	}

	void Application::finish()
	{
		if (this->state != State::Stopped)
		{
			this->state = State::Stopping;
		}
	}

	void Application::finalize()
	{
		if (this->state == State::Stopping)
		{
			this->state = State::Stopped;
			while (this->state == State::Stopped)
			{
				hthread::sleep(0.001f);
			}
		}
	}

	void Application::_asyncUpdate(hthread* thread)
	{
		(*april::application->aprilApplicationInit)();
		april::application->state = State::Running;
		float timeDelta = 0.0f;
		UpdateDelegate* updateDelegate = NULL;
		hmutex::ScopeLock lock;
		while (april::application->state == State::Running)
		{
			lock.acquire(&april::application->updateMutex);
			timeDelta = april::application->timeDelta;
			april::application->timeDelta = 0.0f;
			lock.release();
			if (!april::window->update(timeDelta))
			{
				april::application->finish();
			}
			updateDelegate = april::window->getUpdateDelegate(); // constantly getting to make sure not to use an outdated object
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
			/*
			while (april::application->state == State::Running && april::rendersys->getAsyncQueuesCount() > april::rendersys->getFrameAdvanceUpdates())
			{
				hthread::sleep(0.001f);
			}
			*/
		}
		(*april::application->aprilApplicationDestroy)();
	}

}
