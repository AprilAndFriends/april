/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#ifdef _ANDROID
#include "androidUtilJNI.h"
#endif
#include "april.h"
#include "Application.h"
#include "main_base.h"
#include "RenderSystem.h"
#include "TextureAsync.h"
#include "UpdateDelegate.h"
#include "Window.h"

namespace april
{
#ifdef _ANDROID
	extern void* javaVM;
#endif

	HL_ENUM_CLASS_DEFINE(Application::State,
	(
		HL_ENUM_DEFINE(Application::State, Idle);
		HL_ENUM_DEFINE(Application::State, Starting);
		HL_ENUM_DEFINE(Application::State, Running);
		HL_ENUM_DEFINE(Application::State, Stopping);
		HL_ENUM_DEFINE(Application::State, Stopped);
	));

	Application* application = NULL;
	
	Application::Application(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)()) : state(State::Idle), suspended(false), timeDelta(0.0f),
		fps(0), fpsCount(0), fpsTimer(0.0f), fpsResolution(0.5f), timeDeltaMaxLimit(0.1f), updateThread(&_asyncUpdate, "APRIL Async Update")
	{
		this->aprilApplicationInit = aprilApplicationInit;
		this->aprilApplicationDestroy = aprilApplicationDestroy;
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
			this->_updateSystem();
			hthread::sleep(0.001f);
		}
		this->timer.update();
	}

	void Application::destroy()
	{
		this->state = State::Idle;
		this->resume(); // makes sure the update thread can resume
		this->updateThread.join();
	}

	void Application::enterMainLoop()
	{
		this->fps = 0;
		this->fpsCount = 0;
		this->fpsTimer = 0.0f;
		this->timer.update();
		while (this->state == State::Running)
		{
			this->update();
		}
		this->updateFinishing();
	}

	void Application::updateFinishing()
	{
		// processing remaining commands from other thread
		while (this->state == State::Stopping)
		{
			this->_updateSystem();
		}
		// finish everything up
		this->_updateSystem();
		april::rendersys->_flushAsyncCommands();
		// done
		this->state = State::Idle;
	}

	bool Application::update()
	{
		TextureAsync::update();
		april::window->checkEvents();
		float timeDelta = (float)this->timer.diff(true);
		if (!april::window->isFocused())
		{
			timeDelta = 0.0f;
		}
		this->fpsTimer += timeDelta;
		if (this->timeDeltaMaxLimit > 0.0f)
		{
			timeDelta = hmin(timeDelta, this->timeDeltaMaxLimit);
		}
		hmutex::ScopeLock lock(&this->timeDeltaMutex);
		this->timeDelta += timeDelta;
		lock.release();
		return april::rendersys->update(timeDelta);
	}

	void Application::_updateSystem()
	{
		if (april::window != NULL && april::rendersys != NULL)
		{
			TextureAsync::update();
			april::window->checkEvents();
			april::rendersys->update(0.0f); // might require some rendering
		}
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
#ifdef _ANDROID
		getJNIEnv(); // attaches thread to Java VM
#endif
		if (april::window != NULL && april::rendersys != NULL)
		{
			april::application->state = State::Running;
			float timeDelta = 0.0f;
			UpdateDelegate* updateDelegate = NULL;
			hmutex::ScopeLock lock;
			hmutex::ScopeLock lockTimeDelta;
			while (april::application->state == State::Running)
			{
				lock.acquire(&april::application->updateMutex);
				lockTimeDelta.acquire(&april::application->timeDeltaMutex);
				timeDelta = april::application->timeDelta;
				april::application->timeDelta = 0.0f;
				lockTimeDelta.release();
				if (!april::window->update(timeDelta))
				{
					april::application->finish();
				}
				april::window->setPresentFrameEnabled(false);
				updateDelegate = april::window->getUpdateDelegate(); // constantly getting to make sure not to use an outdated object
				if (updateDelegate != NULL)
				{
					updateDelegate->onPresentFrame();
				}
				else
				{
					april::rendersys->presentFrame();
				}
				april::window->setPresentFrameEnabled(true);
				lock.release();
				while (april::application->state == State::Running && april::rendersys->getAsyncQueuesCount() > april::rendersys->getFrameAdvanceUpdates())
				{
					hthread::sleep(0.001f);
					april::window->_processEvents();
				}
			}
		}
		else
		{
			april::application->state = State::Stopping;
		}
		(*april::application->aprilApplicationDestroy)();
#ifdef _ANDROID
		((JavaVM*)april::javaVM)->DetachCurrentThread();
#endif
	}

	void Application::suspend()
	{
		if (!this->suspended)
		{
			this->updateMutex.lock();
			hlog::write(logTag, "Application suspend.");
			this->suspended = true;
			april::rendersys->_flushAsyncCommands();
		}
	}

	void Application::resume()
	{
		if (this->suspended)
		{
			hlog::write(logTag, "Application resume.");
			this->suspended = false;
			this->updateMutex.unlock();
		}
	}

	void Application::renderFrameSync()
	{
		hmutex::ScopeLock lock(&this->updateMutex);
		// TODO - can this even work?
		//april::rendersys->_repeatLastFrame();
	}

}
